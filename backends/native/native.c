/* The native backend: the seventeen entries of bridge_backend.h, over a shared library.
 * docs/backends/native/native.md is its SDK; docs/seam/backend/capabilities.md is what this
 * backend truthfully cannot do. */
#include "bridge_backend.h"
#include "bridge_native.h"
#include "os.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum entry : uint32_t { DECLARE, SYSTEM, OBSERVE, SPAWN, DESTROY, ATTACH, DETACH, ALLOC, ENTRIES };
static const char *const entry_names[ENTRIES] = {
    "declare", "add_system", "observe", "spawn", "destroy", "attach", "detach", "alloc"
};

struct backend { int unused; };
struct backend_memory { void *ptr; uint64_t len; };
struct backend_module { void *lib; };
struct backend_func { backend_instance_t *instance; int (*fn)(const bridge_api_t *); backend_func_t *next; };

struct backend_instance {
    backend_module_t *m;
    bridge_api_t      api;
    backend_fn_t      fns[ENTRIES];
    void           *uds[ENTRIES];
    uint64_t        calling;        /* the thread a call into it is in progress on, or 0 */
    int             refused;        /* an entry refused during the call in progress */
    backend_func_t   *funcs;
};

/* The table a module was handed is part of its instance, so an entry is told whose it is by
 * the table it is given. Native code runs only inside a call this backend made - native.md,
 * *The invariant* - and that is checked here: an entry reached from any other thread, a foreign
 * thread's callback say, or outside a call, answers zero.
 *
 * A refusal cannot unwind native code, so the entry answers zero and the call returns:
 * Bridge-ASM700 fails the invocation then - signatures.md. Until it does return, every entry
 * answers zero and reaches nothing, which is as near to stopped as native code can be put. */
static uint64_t enter(const bridge_api_t *api, int entry, const uint64_t *args)
{
    auto i = (backend_instance_t *)((char *)api - offsetof(backend_instance_t, api));
    uint64_t result = 0;
    if (!i->calling || i->calling != os_get_thread_id() || i->refused || !i->fns[entry]) return 0;
    if (i->fns[entry](i->uds[entry], args, &result)) {
        i->refused = 1;
        return 0;
    }
    return result;
}

static uint32_t t_declare(const bridge_api_t *api, uint32_t at)
{
    uint64_t args[1] = { at };
    return (uint32_t)enter(api, DECLARE, args);
}

static uint32_t t_system(const bridge_api_t *api, uint32_t at)
{
    uint64_t args[1] = { at };
    return (uint32_t)enter(api, SYSTEM, args);
}

static uint32_t t_observe(const bridge_api_t *api, uint32_t at)
{
    uint64_t args[1] = { at };
    return (uint32_t)enter(api, OBSERVE, args);
}

static uint64_t t_spawn(const bridge_api_t *api)
{
    uint64_t args[1] = {};
    return enter(api, SPAWN, args);
}

static void t_destroy(const bridge_api_t *api, uint64_t id)
{
    enter(api, DESTROY, &id);
}

static void t_attach(const bridge_api_t *api, uint64_t id, uint32_t slot, uint32_t ptr)
{
    uint64_t args[3] = { id, slot, ptr };
    enter(api, ATTACH, args);
}

static void t_detach(const bridge_api_t *api, uint64_t id, uint32_t slot)
{
    uint64_t args[2] = { id, slot };
    enter(api, DETACH, args);
}

static uint32_t t_alloc(const bridge_api_t *api, uint32_t len)
{
    uint64_t args[1] = { len };
    return (uint32_t)enter(api, ALLOC, args);
}

/* ---- the seventeen ---- */

BACKEND_EXPORT backend_t *backend_create(const backend_setting_t *settings, uint32_t count,
                                       backend_err_t *err)
{
    /* this backend has no setting of its own, so any key is one it does not know */
    if (count) {
        snprintf(err->text, sizeof err->text, "it knows no setting '%.100s'",
                 settings[0].key ? settings[0].key : "");
        return nullptr;
    }
    return calloc(1, sizeof(backend_t));
}

BACKEND_EXPORT void backend_destroy(backend_t *b)
{
    free(b);
}

/* It runs only what is trusted, and keeps no work limit, no timeout and no memory limit:
 * nothing contains code in this process. And not
 * side by side: a process cannot hold two copies of one library. */
BACKEND_EXPORT enum backend_capability backend_get_capabilities([[maybe_unused]] backend_t *b)
{
    return BACKEND_RELOAD | BACKEND_UNLOAD;
}

#ifndef SAYS_ABI
#define SAYS_ABI BACKEND_ABI
#endif
BACKEND_EXPORT uint32_t backend_get_abi() { return SAYS_ABI; }

BACKEND_EXPORT void memory_get_shape([[maybe_unused]] backend_t *b, uint64_t *reservation, uint64_t *guard,
                                 uint64_t *granule)
{
    *reservation = 0;
    *guard = 0;
    *granule = 0;
}

/* almost nothing: the pointer is already addressable */
BACKEND_EXPORT backend_memory_t *memory_adopt(
    [[maybe_unused]] backend_t *b, void *ptr, uint64_t len, [[maybe_unused]] backend_err_t *err)
{
    backend_memory_t *m = malloc(sizeof *m);
    m->ptr = ptr;
    m->len = len;
    return m;
}

BACKEND_EXPORT void memory_release(backend_memory_t *m)
{
    free(m);
}

BACKEND_EXPORT backend_module_t *module_compile(
    [[maybe_unused]] backend_t *b, [[maybe_unused]] const void *bytes, [[maybe_unused]] size_t len,
    const char *path, [[maybe_unused]] enum backend_capability demands, backend_err_t *err)
{
    backend_module_t *m;
    void *lib;
    /* a native module is its library */
    if (!path || !*path) {
        snprintf(err->text, sizeof err->text, "this backend opens a module's library by its "
            "path, and was handed none");
        return nullptr;
    }
    lib = os_lib_open(path);
    if (!lib) {
        snprintf(err->text, sizeof err->text, "cannot open the library '%.180s'", path);
        return nullptr;
    }
    m = calloc(1, sizeof *m);
    m->lib = lib;
    return m;
}

/* A library has no import section to read, so it is taken to want both memories. */
BACKEND_EXPORT uint32_t module_list_imports([[maybe_unused]] backend_module_t *m, backend_import_t *out,
                                       uint32_t max)
{
    static const char ns[] = BRIDGE_NAMESPACE;
    static const char *const fields[2] = { BRIDGE_SHARED, BRIDGE_PRIVATE };
    uint32_t i;
    for (i = 0; i < 2 && i < max; i ++) {
        out[i].ns = ns;
        out[i].ns_len = sizeof ns - 1;
        out[i].field = fields[i];
        out[i].field_len = strlen(fields[i]);
        out[i].kind = BACKEND_IMPORT_MEMORY;
    }
    return 2;
}

BACKEND_EXPORT void module_destroy(backend_module_t *m)
{
    os_lib_close(m->lib);
    free(m);
}

BACKEND_EXPORT backend_instance_t *instance_create([[maybe_unused]] backend_t *b, backend_module_t *m,
                                                 const backend_memory_import_t *memories,
                                                 uint32_t count, [[maybe_unused]] backend_err_t *err)
{
    backend_instance_t *i = calloc(1, sizeof *i);
    uint32_t k;
    i->m = m;
    i->api.declare = t_declare;
    i->api.add_system = t_system;
    i->api.observe = t_observe;
    i->api.spawn = t_spawn;
    i->api.destroy = t_destroy;
    i->api.attach = t_attach;
    i->api.detach = t_detach;
    i->api.alloc = t_alloc;
    for (k = 0; k < count; k ++) {
        if (!strcmp(memories[k].field, BRIDGE_SHARED)) i->api.shared_base = memories[k].memory->ptr;
        if (!strcmp(memories[k].field, BRIDGE_PRIVATE)) i->api.private_base = memories[k].memory->ptr;
    }
    return i;
}

BACKEND_EXPORT void instance_destroy(backend_instance_t *i)
{
    while (i->funcs) {
        backend_func_t *f = i->funcs;
        i->funcs = f->next;
        free(f);
    }
    free(i);
}

/* this backend answered no to all three, so Bridge-ASM700 never has a number to hand them */
BACKEND_EXPORT void instance_set_work_limit([[maybe_unused]] backend_instance_t *i,
                                      [[maybe_unused]] uint64_t units) { }
BACKEND_EXPORT void instance_set_timeout(
    [[maybe_unused]] backend_instance_t *i, [[maybe_unused]] uint64_t nanos) { }
BACKEND_EXPORT void instance_set_memory_limit(
    [[maybe_unused]] backend_instance_t *i, [[maybe_unused]] uint64_t bytes) { }

/* The signature is the one bridge_native.h already declares, so only the pointer is kept. */
BACKEND_EXPORT int define_func(backend_instance_t *i, const char *ns, const char *field,
                               backend_fn_t fn, void *ud, [[maybe_unused]] const backend_sig_t *sig,
                               backend_err_t *err)
{
    uint32_t k;
    for (k = 0; k < ENTRIES; k ++) {
        if (!strcmp(ns, BRIDGE_NAMESPACE) && !strcmp(field, entry_names[k])) {
            i->fns[k] = fn;
            i->uds[k] = ud;
            return 0;
        }
    }
    snprintf(err->text, sizeof err->text, "the native SDK has no entry '%.100s'", field);
    return -1;
}

/* Exports are named bridge_<name>: a C process has one flat symbol namespace. */
/* A symbol carries no type, so there is no shape to see: what it must be is native.md's. */
BACKEND_EXPORT backend_func_t *instance_get_func(backend_instance_t *i, const char *name, backend_err_t *err)
{
    char symbol[160];
    backend_func_t *f;
    int (*fn)(const bridge_api_t *);
    snprintf(symbol, sizeof symbol, "bridge_%.150s", name);
    fn = (int (*)(const bridge_api_t *))os_lib_get_symbol(i->m->lib, symbol);
    if (!fn) {
        snprintf(err->text, sizeof err->text, "no exported function '%s'", symbol);
        return nullptr;
    }
    f = calloc(1, sizeof *f);
    f->instance = i;
    f->fn = fn;
    f->next = i->funcs;
    i->funcs = f;
    return f;
}

/* Nothing is caught: a native module that faults takes the process with it. */
BACKEND_EXPORT int call(backend_func_t *f, int32_t *status, [[maybe_unused]] backend_err_t *err)
{
    backend_instance_t *i = f->instance;
    uint64_t before = i->calling;       /* a call made from inside a call returns to it */
    i->calling = os_get_thread_id();
    i->refused = 0;
    *status = f->fn(&i->api);
    i->calling = before;
    return 0;
}
