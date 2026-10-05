/* The wasm backend: the seventeen entries of bridge-backend.h, over wasmtime.
 * docs/seam/backend/impl/wasmtime.md is why it is shaped this way. */
#include "bridge-backend.h"
#include "os.h"

#include <wasm.h>
#include <wasmtime.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* wasmtime enforces a 32-bit memory's bounds by faulting: the whole 4 GiB an index can
 * reach is reserved, then a guard, and loads are emitted with no check. Stated here and
 * to wasmtime both, so that what memory_shape answers is what the engine assumes. */
#define RESERVATION ((uint64_t)1 << 32)
#define GUARD       ((uint64_t)32 << 20)
#define PAGE        65536u
/* a deadline no call reaches: every store needs one once epochs are on */
#define FAR         ((uint64_t)1 << 62)

#ifdef _MSC_VER
#define THREAD_LOCAL __declspec(thread)
#else
#define THREAD_LOCAL _Thread_local
#endif

/* Fuel and epoch interruption are an engine's settings and put code in every loop, so there
 * is an engine per combination a module can demand, made at its first use. */
enum { METERED = 1, INTERRUPTIBLE = 2, ENGINES = 4 };

struct backend {
    wasm_engine_t *engines[ENGINES];
    wasm_engine_t *ticking;         /* the engine of the call the watchdog is armed for */
    os_thread *watchdog;
    os_event  *armed, *done;
    uint64_t   nanos;
    int        quit;
};

struct backend_memory { void *ptr; uint64_t len; };

struct backend_module {
    backend *b;
    int kind;                       /* METERED | INTERRUPTIBLE */
    wasmtime_module_t *module;
    wasm_importtype_vec_t imports;
};

struct backend_func {
    backend_instance *instance;
    wasmtime_func_t   func;
    backend_func     *next;
};

struct backend_instance {
    backend *b;
    backend_module *m;
    wasm_engine_t *engine;
    wasmtime_store_t   *store;
    wasmtime_context_t *ctx;
    wasmtime_linker_t  *linker;
    wasmtime_instance_t instance;
    int      instantiated;
    uint64_t deadline;          /* nanoseconds, for the next call only */
    backend_func *funcs;
};

/* wasmtime_error_delete takes ownership of a real error: it must not be handed NULL */
static void drop(wasmtime_error_t *error)
{
    if (error) wasmtime_error_delete(error);
}

static void fail(backend_err *err, const char *what, wasmtime_error_t *error, wasm_trap_t *trap)
{
    wasm_byte_vec_t msg = {0};
    if (error) wasmtime_error_message(error, &msg);
    else if (trap) wasm_trap_message(trap, &msg);
    if (err) {
        /* wasmtime leads with a backtrace and ends with the reason; the reason is what a
         * record should say */
        static const char marker[] = "wasm trap: ";
        const char *text = msg.data, *end = msg.data + msg.size, *p, *line;
        size_t n;
        for (p = text; p && p + sizeof marker - 1 <= end; p ++) {
            if (!memcmp(p, marker, sizeof marker - 1)) text = p + sizeof marker - 1;
        }
        line = text ? memchr(text, '\n', (size_t)(end - text)) : NULL;
        n = text ? (size_t)((line ? line : end) - text) : 0;
        snprintf(err->text, sizeof err->text, "%s%s%.*s", what, n ? ": " : "",
                 (int)(n > 200 ? 200 : n), text ? text : "");
    }
    if (msg.data) wasm_byte_vec_delete(&msg);
    if (error) wasmtime_error_delete(error);
    if (trap) wasm_trap_delete(trap);
}

/* ---- memory: wasmtime asks this for every linear memory, and is not told which ---- */

/* set around wasmtime_memory_new: the creator is being asked for a block Bridge-ASM700 owns */
static THREAD_LOCAL backend_memory *adopting;

typedef struct { uint8_t *base; size_t size, reserve; } own_memory;

static uint8_t *adopted_get(void *env, size_t *size, size_t *capacity)
{
    backend_memory *m = env;
    *size = *capacity = (size_t)m->len;
    return m->ptr;
}

static wasmtime_error_t *adopted_grow(void *env, size_t new_size)
{
    backend_memory *m = env;
    return new_size == m->len ? NULL : wasmtime_error_new("an adopted memory does not grow");
}

static uint8_t *own_get(void *env, size_t *size, size_t *capacity)
{
    own_memory *m = env;
    *size = m->size;
    *capacity = m->reserve;
    return m->base;
}

static wasmtime_error_t *own_grow(void *env, size_t new_size)
{
    own_memory *m = env;
    if (new_size > m->reserve || os_commit(m->base, new_size)) {
        return wasmtime_error_new("out of address space for a module's own memory");
    }
    m->size = new_size;
    return NULL;
}

static void own_free(void *env)
{
    own_memory *m = env;
    os_release(m->base, m->reserve + (size_t)GUARD);
    free(m);
}

static wasmtime_error_t *new_memory(void *env, const wasm_memorytype_t *ty, size_t minimum,
                                    size_t maximum, size_t reserved, size_t guard,
                                    wasmtime_linear_memory_t *ret)
{
    (void)env; (void)ty; (void)maximum;
    if (reserved > RESERVATION || guard > GUARD) {
        return wasmtime_error_new("wasmtime asked for more address space than memory_shape states");
    }
    if (adopting) {
        if (minimum != adopting->len) {
            return wasmtime_error_new("an adopted block is not the size its import declares");
        }
        ret->env = adopting;
        ret->get_memory = adopted_get;
        ret->grow_memory = adopted_grow;
        ret->finalizer = NULL;              /* the block is Bridge-ASM700's to free */
        return NULL;
    }
    own_memory *m = malloc(sizeof *m);
    m->reserve = (size_t)RESERVATION;
    m->size = minimum;
    m->base = os_reserve((size_t)(RESERVATION + GUARD), minimum);
    if (!m->base) {
        free(m);
        return wasmtime_error_new("out of address space for a module's own memory");
    }
    ret->env = m;
    ret->get_memory = own_get;
    ret->grow_memory = own_grow;
    ret->finalizer = own_free;
    return NULL;
}

/* ---- the clock: one armed deadline at a time, and no tick period ---- */

static void watchdog(void *arg)
{
    backend *b = arg;
    for (;;) {
        os_event_wait(b->armed, OS_FOREVER);
        if (b->quit) return;
        if (!os_event_wait(b->done, b->nanos)) {
            wasmtime_engine_increment_epoch(b->ticking);
            os_event_wait(b->done, OS_FOREVER);     /* the call does return, having trapped */
        }
    }
}

static wasm_engine_t *engine_for(backend *b, int kind)
{
    if (!b->engines[kind]) {
        wasm_config_t *config = wasm_config_new();
        wasmtime_memory_creator_t creator = { NULL, new_memory, NULL };
        wasmtime_config_wasm_multi_memory_set(config, true);
        wasmtime_config_consume_fuel_set(config, (kind & METERED) != 0);
        wasmtime_config_epoch_interruption_set(config, (kind & INTERRUPTIBLE) != 0);
        wasmtime_config_memory_reservation_set(config, RESERVATION);
        wasmtime_config_memory_guard_size_set(config, GUARD);
        wasmtime_config_memory_may_move_set(config, false);
        wasmtime_config_host_memory_creator_set(config, &creator);
        b->engines[kind] = wasm_engine_new_with_config(config);
    }
    return b->engines[kind];
}

/* ---- the seventeen ---- */

BACKEND_EXPORT backend *backend_create(const backend_setting *settings, uint32_t count,
                                       backend_err *err)
{
    (void)settings; (void)count; (void)err;
    backend *b = calloc(1, sizeof *b);
    b->armed = os_event_new();
    b->done = os_event_new();
    b->watchdog = os_thread_start(watchdog, b);
    return b;
}

BACKEND_EXPORT void backend_destroy(backend *b)
{
    b->quit = 1;
    os_event_set(b->armed);
    os_thread_join(b->watchdog);
    os_event_free(b->armed);
    os_event_free(b->done);
    for (int k = 0; k < ENGINES; k ++) {
        if (b->engines[k]) wasm_engine_delete(b->engines[k]);
    }
    free(b);
}

BACKEND_EXPORT uint32_t backend_capabilities(backend *b)
{
    (void)b;
    return BACKEND_SANDBOX | BACKEND_METER | BACKEND_DEADLINE | BACKEND_LIMIT | BACKEND_RELOAD;
}

BACKEND_EXPORT void memory_shape(backend *b, uint64_t *reservation, uint64_t *guard)
{
    (void)b;
    *reservation = RESERVATION;
    *guard = GUARD;
}

BACKEND_EXPORT backend_memory *memory_adopt(backend *b, void *ptr, uint64_t len, backend_err *err)
{
    (void)b;
    if (!ptr || !len || len % PAGE) {
        fail(err, "an adopted block must be a whole number of 64 KiB pages", NULL, NULL);
        return NULL;
    }
    backend_memory *m = malloc(sizeof *m);
    m->ptr = ptr;
    m->len = len;
    return m;
}

BACKEND_EXPORT void memory_release(backend_memory *m)
{
    free(m);
}

BACKEND_EXPORT backend_module *module_compile(backend *b, const void *bytes, size_t len,
                                              const char *path, uint32_t demands,
                                              backend_err *err)
{
    (void)path;                 /* a wasm module is its bytes */
    int kind = (demands & BACKEND_METER ? METERED : 0)
             | (demands & BACKEND_DEADLINE ? INTERRUPTIBLE : 0);
    wasmtime_module_t *module = NULL;
    wasmtime_error_t *error = wasmtime_module_new(engine_for(b, kind), bytes, len, &module);
    if (error) {
        fail(err, "not a module this backend can compile", error, NULL);
        return NULL;
    }
    backend_module *m = calloc(1, sizeof *m);
    m->b = b;
    m->kind = kind;
    m->module = module;
    wasmtime_module_imports(module, &m->imports);
    return m;
}

BACKEND_EXPORT uint32_t module_imports(backend_module *m, backend_import *out, uint32_t max)
{
    uint32_t i, n = (uint32_t)m->imports.size;
    for (i = 0; i < n && i < max; i ++) {
        const wasm_importtype_t *it = m->imports.data[i];
        const wasm_name_t *ns = wasm_importtype_module(it), *field = wasm_importtype_name(it);
        wasm_externkind_t kind = wasm_externtype_kind(wasm_importtype_type(it));
        out[i].ns = ns->data;       out[i].ns_len = ns->size;
        out[i].field = field->data; out[i].field_len = field->size;
        out[i].kind = kind == WASM_EXTERN_FUNC ? BACKEND_IMPORT_FUNC
                    : kind == WASM_EXTERN_MEMORY ? BACKEND_IMPORT_MEMORY : BACKEND_IMPORT_OTHER;
    }
    return n;
}

BACKEND_EXPORT void module_destroy(backend_module *m)
{
    wasm_importtype_vec_delete(&m->imports);
    wasmtime_module_delete(m->module);
    free(m);
}

BACKEND_EXPORT void instance_destroy(backend_instance *i)
{
    while (i->funcs) {
        backend_func *f = i->funcs;
        i->funcs = f->next;
        free(f);
    }
    wasmtime_linker_delete(i->linker);
    wasmtime_store_delete(i->store);
    free(i);
}

static size_t leb(uint8_t *out, uint64_t v)
{
    size_t n = 0;
    do {
        uint8_t byte = v & 0x7f;
        v >>= 7;
        out[n ++] = (uint8_t)(byte | (v ? 0x80 : 0));
    } while (v);
    return n;
}

/* wasmtime asks the creator only for a memory an instance defines, never for one the host
 * creates. So an adopted block enters a store as the memory of a module that does nothing
 * but define one and export it, and the creator answers for that with the block. */
static wasmtime_error_t *adopt_into(backend_instance *i, backend_memory *mem,
                                    wasmtime_extern_t *item, wasm_trap_t **trap)
{
    uint8_t wasm[64], limits[24];
    uint64_t pages = mem->len / PAGE;
    size_t n = 8, l = 0;
    wasmtime_module_t *holder = NULL;
    wasmtime_instance_t instance;
    wasmtime_error_t *error;

    limits[l ++] = 1;                               /* one memory */
    limits[l ++] = 1;                               /* with a maximum, which is its minimum */
    l += leb(limits + l, pages);
    l += leb(limits + l, pages);
    memcpy(wasm, "\0asm\1\0\0\0", 8);
    wasm[n ++] = 5;                                 /* the memory section */
    wasm[n ++] = (uint8_t)l;
    memcpy(wasm + n, limits, l);
    n += l;
    memcpy(wasm + n, "\7\5\1\1m\2\0", 7);           /* the export section: "m", memory 0 */
    n += 7;

    error = wasmtime_module_new(i->engine, wasm, n, &holder);
    if (error) return error;
    adopting = mem;
    error = wasmtime_instance_new(i->ctx, holder, NULL, 0, &instance, trap);
    adopting = NULL;
    wasmtime_module_delete(holder);
    if (error || *trap) return error;
    if (!wasmtime_instance_export_get(i->ctx, &instance, "m", 1, item)) {
        return wasmtime_error_new("the holder module exports no memory");
    }
    return NULL;
}

BACKEND_EXPORT backend_instance *instance_create(backend *b, backend_module *m,
                                                 const backend_memory_import *memories,
                                                 uint32_t count, backend_err *err)
{
    backend_instance *i = calloc(1, sizeof *i);
    uint32_t k;
    i->b = b;
    i->m = m;
    i->engine = engine_for(b, m->kind);
    i->store = wasmtime_store_new(i->engine, NULL, NULL);
    i->ctx = wasmtime_store_context(i->store);
    i->linker = wasmtime_linker_new(i->engine);
    /* a metered store with no fuel traps at its first instruction */
    if (m->kind & METERED) drop(wasmtime_context_set_fuel(i->ctx, UINT64_MAX));
    /* and an interruptible one with no deadline at the first increment: the holder below runs
     * nothing, but it is instantiated in this store */
    if (m->kind & INTERRUPTIBLE) wasmtime_context_set_epoch_deadline(i->ctx, FAR);

    for (k = 0; k < count; k ++) {
        const backend_memory_import *mi = &memories[k];
        wasmtime_extern_t item;
        wasm_trap_t *trap = NULL;
        wasmtime_error_t *error = adopt_into(i, mi->memory, &item, &trap);
        if (trap) {
            fail(err, "cannot give a module the memory it imports", NULL, trap);
            instance_destroy(i);
            return NULL;
        }
        if (!error) {
            error = wasmtime_linker_define(i->linker, i->ctx, mi->ns, strlen(mi->ns),
                                           mi->field, strlen(mi->field), &item);
        }
        if (error) {
            fail(err, "cannot give a module the memory it imports", error, NULL);
            instance_destroy(i);
            return NULL;
        }
    }
    return i;
}

BACKEND_EXPORT void instance_set_fuel(backend_instance *i, uint64_t units)
{
    if (i->m->kind & METERED) drop(wasmtime_context_set_fuel(i->ctx, units));
}

BACKEND_EXPORT void instance_set_deadline(backend_instance *i, uint64_t nanos)
{
    i->deadline = nanos;
}

BACKEND_EXPORT void instance_set_memory_limit(backend_instance *i, uint64_t bytes)
{
    wasmtime_store_limiter(i->store, (int64_t)bytes, -1, -1, -1, -1);
}

/* ---- functions in both directions ---- */

typedef struct { backend_fn fn; void *ud; uint32_t params, results; uint8_t result_kind[4]; } host_fn;

static wasm_trap_t *host_call(void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args,
                              size_t nargs, wasmtime_val_t *results, size_t nresults)
{
    host_fn *h = env;
    uint64_t in[16] = {0}, out[4] = {0};
    size_t k;
    (void)caller;
    for (k = 0; k < nargs && k < 16; k ++) {
        in[k] = args[k].kind == WASMTIME_I32 ? (uint32_t)args[k].of.i32 : (uint64_t)args[k].of.i64;
    }
    if (h->fn(h->ud, in, out)) {
        /* Bridge-ASM700 knows why and says so itself: the trap only has to unwind */
        static const char refused[] = "refused by Bridge-ASM700";
        return wasmtime_trap_new(refused, sizeof refused - 1);
    }
    for (k = 0; k < nresults && k < 4; k ++) {
        if (h->result_kind[k] == BACKEND_I32) {
            results[k].kind = WASMTIME_I32;
            results[k].of.i32 = (int32_t)out[k];
        } else {
            results[k].kind = WASMTIME_I64;
            results[k].of.i64 = (int64_t)out[k];
        }
    }
    return NULL;
}

static void types(wasm_valtype_vec_t *vec, const uint8_t *kinds, uint32_t count)
{
    uint32_t k;
    wasm_valtype_vec_new_uninitialized(vec, count);
    for (k = 0; k < count; k ++) {
        vec->data[k] = wasm_valtype_new(kinds[k] == BACKEND_I32 ? WASM_I32 : WASM_I64);
    }
}

BACKEND_EXPORT int define_func(backend_instance *i, const char *ns, const char *field,
                               backend_fn fn, void *ud, const backend_sig *sig, backend_err *err)
{
    wasm_valtype_vec_t params, results;
    host_fn *h;
    if (i->instantiated || sig->param_count > 16 || sig->result_count > 4) {
        fail(err, "define_func: after the module's first function, or too wide", NULL, NULL);
        return -1;
    }
    h = calloc(1, sizeof *h);
    h->fn = fn;
    h->ud = ud;
    memcpy(h->result_kind, sig->results, sig->result_count);
    types(&params, sig->params, sig->param_count);
    types(&results, sig->results, sig->result_count);
    wasm_functype_t *ty = wasm_functype_new(&params, &results);
    wasmtime_error_t *error = wasmtime_linker_define_func(i->linker, ns, strlen(ns), field,
                                                          strlen(field), ty, host_call, h, free);
    wasm_functype_delete(ty);
    if (error) {
        fail(err, "cannot define a function", error, NULL);
        return -1;
    }
    return 0;
}

/* a module is instantiated at its first function: every import has to exist by then */
static int instantiate(backend_instance *i, backend_err *err)
{
    wasm_trap_t *trap = NULL;
    wasmtime_error_t *error;
    if (i->instantiated) return 0;
    error = wasmtime_linker_instantiate(i->linker, i->ctx, i->m->module, &i->instance, &trap);
    if (error || trap) {
        fail(err, "cannot instantiate", error, trap);
        return -1;
    }
    i->instantiated = 1;
    return 0;
}

BACKEND_EXPORT backend_func *instance_func(backend_instance *i, const char *name,
                                           int32_t *arity, int32_t *results, backend_err *err)
{
    wasmtime_extern_t item;
    wasm_functype_t *ty;
    backend_func *f;
    if (instantiate(i, err)) return NULL;
    if (!wasmtime_instance_export_get(i->ctx, &i->instance, name, strlen(name), &item)
        || item.kind != WASMTIME_EXTERN_FUNC) {
        if (err) snprintf(err->text, sizeof err->text, "no exported function '%s'", name);
        return NULL;
    }
    ty = wasmtime_func_type(i->ctx, &item.of.func);
    *arity = (int32_t)wasm_functype_params(ty)->size;
    *results = (int32_t)wasm_functype_results(ty)->size;
    /* a result that is not an i32 is as wrong a shape as a second one */
    if (*results == 1 && wasm_valtype_kind(wasm_functype_results(ty)->data[0]) != WASM_I32) {
        *results = 2;
    }
    wasm_functype_delete(ty);

    f = calloc(1, sizeof *f);
    f->instance = i;
    f->func = item.of.func;
    f->next = i->funcs;
    i->funcs = f;
    return f;
}

BACKEND_EXPORT int call(backend_func *f, int32_t *status, backend_err *err)
{
    backend_instance *i = f->instance;
    backend *b = i->b;
    wasmtime_val_t result;
    wasm_trap_t *trap = NULL;
    wasmtime_error_t *error;
    uint64_t nanos = i->deadline;
    i->deadline = 0;

    /* a module compiled without the check cannot be interrupted, and was not asked to be */
    if (!(i->m->kind & INTERRUPTIBLE)) nanos = 0;
    if (nanos) {
        wasmtime_context_set_epoch_deadline(i->ctx, 1);
        b->nanos = nanos;
        b->ticking = i->engine;
        os_event_set(b->armed);
    } else if (i->m->kind & INTERRUPTIBLE) {
        wasmtime_context_set_epoch_deadline(i->ctx, FAR);
    }
    error = wasmtime_func_call(i->ctx, &f->func, NULL, 0, &result, 1, &trap);
    if (nanos) os_event_set(b->done);

    if (error || trap) {
        fail(err, "trapped", error, trap);
        return -1;
    }
    *status = result.of.i32;
    return 0;
}
