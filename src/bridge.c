/* Bridge-ASM700, as far as phase 3: open the backends, build storage, load a module through a
 * backend, run a phase, and say why when any of it will not.
 * docs/seam/host-api/create.md, lifecycle.md and loop.md. */
#include "inside.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* A record of Bridge-ASM700's own. `m` and `s` say whose it is about, when it is anybody's. */
void say(bridge *b, const module *m, const bridge_system *s, int8_t level, const char *fmt, ...)
{
    char text[512];
    bridge_record r = {0};
    va_list args;
    va_start(args, fmt);
    vsnprintf(text, sizeof text, fmt, args);
    va_end(args);
    r.identity = m ? m->identity : NULL;
    r.system = s ? s->name : NULL;
    r.text = text;
    r.len = (uint32_t)strlen(text);
    r.when = b->barriers << 16;
    r.level = level;
    b->sink(b->ud, &r);
}

char *copy(const char *s, size_t n)
{
    char *c = malloc(n + 1);
    memcpy(c, s, n);
    c[n] = 0;
    return c;
}

static char **copy_list(const char *const *list, uint32_t count)
{
    char **out = calloc(count ? count : 1, sizeof *out);
    uint32_t i;
    for (i = 0; i < count; i ++) out[i] = copy(list[i], strlen(list[i]));
    return out;
}

static void free_list_of(char **list, uint32_t count)
{
    uint32_t i;
    for (i = 0; list && i < count; i ++) free(list[i]);
    free(list);
}

/* Take a module out: its systems, its instance, its image, its private memory. The order is
 * the backend seam's: a memory is released after the last instance it was handed to. */
static void discard(bridge *b, module *m)
{
    uint32_t i, kept = 0;
    for (i = 0; i < b->system_count; i ++) {
        bridge_system *s = b->systems[i];
        if (s->m != m) {
            b->systems[kept ++] = s;
            continue;
        }
        if (s->query) ecs_query_fini(s->query);
        free(s->name);
        free(s);
    }
    b->system_count = kept;
    if (m->instance) m->on->api.instance_destroy(m->instance);
    if (m->image) m->on->api.module_destroy(m->image);
    if (m->memory) m->on->api.memory_release(m->memory);
    if (m->priv) os_release(m->priv, m->priv_reserve);
    free_list_of(m->read, m->read_count);
    free_list_of(m->write, m->write_count);
    free(m->systems);
    free(m->declared);
    free(m->identity);
    free(m->ns);
    free(m);
}

void bridge_destroy(bridge *b)
{
    uint32_t i;
    if (!b) return;
    while (b->modules) {
        module *m = b->modules;
        b->modules = m->next;
        discard(b, m);
    }
    free(b->systems);
    store_destroy(b);
    for (i = 0; i < b->backend_count; i ++) {
        open_backend *o = &b->backends[i];
        if (o->be) o->api.backend_destroy(o->be);
        if (o->lib) os_lib_close(o->lib);
        free(o->name);
    }
    for (i = 0; i < b->phase_count; i ++) free(b->phases[i]);
    free(b->phases);
    free(b->backends);
    free(b->stopped_by);
    free(b);
}

bridge *bridge_create(const bridge_config *cfg, bridge_sink_fn sink, void *ud)
{
    bridge *b;
    uint32_t i, k;
    /* the one failure no record can explain, and a host that passed null already knows */
    if (!sink || !cfg) return NULL;

    b = calloc(1, sizeof *b);
    b->sink = sink;
    b->ud = ud;
    b->backends = calloc(cfg->backend_count ? cfg->backend_count : 1, sizeof *b->backends);
    b->phases = calloc(cfg->phase_count ? cfg->phase_count : 1, sizeof *b->phases);

    for (i = 0; i < cfg->backend_count; i ++) {
        const bridge_backend *want = &cfg->backends[i];
        open_backend *o = &b->backends[i];
        char why[400];
        backend_err err = {0};
        uint64_t reservation = 0, guard = 0;

        if (!want->name || !want->library) {
            say(b, NULL, NULL, BRIDGE_ERR, "backend %u has no %s", i,
                want->name ? "library" : "name");
            goto refuse;
        }
        for (k = 0; k < i; k ++) {
            if (!strcmp(b->backends[k].name, want->name)) {
                say(b, NULL, NULL, BRIDGE_ERR, "backend '%s' is named twice", want->name);
                goto refuse;
            }
        }
        o->name = copy(want->name, strlen(want->name));
        b->backend_count = i + 1;
        o->lib = backend_open(want->library, &o->api, why, sizeof why);
        if (!o->lib) {
            say(b, NULL, NULL, BRIDGE_ERR, "backend '%s': %s", want->name, why);
            goto refuse;
        }
        o->be = o->api.backend_create(NULL, 0, &err);
        if (!o->be) {
            say(b, NULL, NULL, BRIDGE_ERR, "backend '%s' would not start: %s", want->name, err.text);
            goto refuse;
        }
        o->capabilities = o->api.backend_capabilities(o->be);
        o->api.memory_shape(o->be, &reservation, &guard);
        if (reservation > b->reservation) b->reservation = reservation;
        if (guard > b->guard) b->guard = guard;
    }

    /* schedule.md, *Bridge-ASM700 ships no phase names*: a deployment declares its own */
    if (!cfg->phase_count) {
        say(b, NULL, NULL, BRIDGE_ERR, "the deployment declares no phase, and there is none "
            "of Bridge-ASM700's to run");
        goto refuse;
    }
    for (i = 0; i < cfg->phase_count; i ++) {
        const char *name = cfg->phases[i];
        if (!name || !*name) {
            say(b, NULL, NULL, BRIDGE_ERR, "phase %u has no name", i);
            goto refuse;
        }
        for (k = 0; k < i; k ++) {
            if (!strcmp(b->phases[k], name)) {
                say(b, NULL, NULL, BRIDGE_ERR, "phase '%s' is named twice", name);
                goto refuse;
            }
        }
        b->phases[i] = copy(name, strlen(name));
        b->phase_count = i + 1;
    }

    if (store_create(b, &cfg->budget)) goto refuse;
    return b;

refuse:
    bridge_destroy(b);
    return NULL;
}

/* A key whose zero means nobody stated it - config.md, *No unaccountable values*. */
static int unset(bridge *b, const char *identity, const char *key, int64_t value)
{
    if (value) return 0;
    say(b, NULL, NULL, BRIDGE_ERR, "module '%s' is refused: %s is unset", identity, key);
    return 1;
}

/* A stated limit demands that the backend can enforce it - capabilities.md. */
static int lacks(bridge *b, const char *identity, open_backend *o, uint32_t flag,
                 const char *key, const char *capability)
{
    if (o->capabilities & flag) return 0;
    say(b, NULL, NULL, BRIDGE_ERR, "module '%s' is refused: %s needs '%s', which backend '%s' "
        "lacks", identity, key, capability, o->name);
    return 1;
}

static int stopped(bridge *b)
{
    if (!b->stopped_by) return 0;
    say(b, NULL, NULL, BRIDGE_ERR, "the bridge has stopped: %s", b->stopped_by);
    return 1;
}

/* The private block: the header at 0, then the three regions its config sized. */
static int private_memory(bridge *b, module *m, const bridge_module_config *cfg, backend_err *err)
{
    uint64_t scratch = 128, ring, spans, len, reserve;
    ring = (scratch + cfg->scratch + 7u) & ~7ull;
    spans = (ring + cfg->log_ring + 7u) & ~7ull;
    len = (spans + cfg->spans + PAGE - 1) & ~(uint64_t)(PAGE - 1);
    reserve = (len > b->reservation ? len : b->reservation) + b->guard;
    m->priv = os_reserve((size_t)reserve, (size_t)len);
    if (!m->priv) {
        snprintf(err->text, sizeof err->text, "the system would not give its private memory "
            "of %llu bytes", (unsigned long long)len);
        return 1;
    }
    m->priv_len = (size_t)len;
    m->priv_reserve = (size_t)reserve;
    m->scratch = (uint32_t)scratch;
    m->scratch_len = cfg->scratch;
    m->ring = (uint32_t)ring;
    m->ring_len = cfg->log_ring / (uint32_t)sizeof(bridge_log_record);
    m->spans = (uint32_t)spans;
    m->spans_len = cfg->spans;
    ((bridge_header *)m->priv)->log_level = cfg->log_level;
    ((bridge_header *)m->priv)->system = UINT32_MAX;        /* no system is running */
    m->memory = m->on->api.memory_adopt(m->on->be, m->priv, len, err);
    return !m->memory;
}

int bridge_load(bridge *b, const bridge_artifact *a)
{
    const bridge_module_config *cfg = &a->cfg;
    const char *id = cfg->identity, *colon;
    open_backend *o = NULL;
    backend_func *init;
    backend_err err = {0};
    backend_import imports[64];
    backend_memory_import memories[2];
    int32_t arity = 0, results = 0, status = 0;
    uint32_t i, demands, import_count, memory_count = 0;
    module *m, *other;

    if (stopped(b)) return 1;
    if (!id || !*id) {
        say(b, NULL, NULL, BRIDGE_ERR, "a module with no identity is refused");
        return 1;
    }

    /* everything that can be decided from cfg alone is decided before the artifact is touched */
    for (i = 0; i < b->backend_count && a->backend; i ++) {
        if (!strcmp(b->backends[i].name, a->backend)) o = &b->backends[i];
    }
    if (!o) {
        say(b, NULL, NULL, BRIDGE_ERR, "module '%s' is refused: no backend named '%s'", id,
            a->backend ? a->backend : "");
        return 1;
    }
    if (cfg->abi != BRIDGE_ABI) {
        say(b, NULL, NULL, BRIDGE_ERR, "module '%s' is refused: it wants ABI %u and this is "
            "ABI %u", id, cfg->abi, (unsigned)BRIDGE_ABI);
        return 1;
    }
    if (cfg->on_error != BRIDGE_FAIL && cfg->on_error != BRIDGE_SKIP) {
        say(b, NULL, NULL, BRIDGE_ERR, "module '%s' is refused: on_error is unset", id);
        return 1;
    }
    if (unset(b, id, "fuel", cfg->fuel) || unset(b, id, "memory_limit", cfg->memory_limit)
        || unset(b, id, "init_timeout", cfg->init_timeout) || unset(b, id, "scratch", cfg->scratch)
        || unset(b, id, "log_ring", cfg->log_ring) || unset(b, id, "spans", cfg->spans)) return 1;
    /* budgets.md: the one floor with an argument. A module that can report nothing is the one
     * legal number that makes a deployment incoherent. */
    if (cfg->log_ring < sizeof(bridge_log_record)) {
        say(b, NULL, NULL, BRIDGE_ERR, "module '%s' is refused: a log_ring of %u bytes holds "
            "no record, and one is %u", id, cfg->log_ring, (unsigned)sizeof(bridge_log_record));
        return 1;
    }
    if ((!cfg->trusted && lacks(b, id, o, BACKEND_SANDBOX, "trusted = false", "sandbox"))
        || (cfg->fuel > 0 && lacks(b, id, o, BACKEND_METER, "fuel", "meter"))
        || (cfg->memory_limit > 0 && lacks(b, id, o, BACKEND_LIMIT, "memory_limit", "limit"))
        || (cfg->init_timeout > 0 && lacks(b, id, o, BACKEND_DEADLINE, "init_timeout", "deadline"))) {
        return 1;
    }
    for (other = b->modules; other; other = other->next) {
        if (!strcmp(other->identity, id)) {
            say(b, NULL, NULL, BRIDGE_ERR, "module '%s' is refused: it is loaded already", id);
            return 1;
        }
    }
    if (b->module_count >= b->max_modules) {
        say(b, NULL, NULL, BRIDGE_ERR, "module '%s' is refused: [budget] max_modules of %u is "
            "reached", id, b->max_modules);
        return 1;
    }
    demands = (cfg->trusted ? 0 : BACKEND_SANDBOX) | (cfg->fuel > 0 ? BACKEND_METER : 0)
            | (cfg->memory_limit > 0 ? BACKEND_LIMIT : 0)
            | (cfg->init_timeout > 0 ? BACKEND_DEADLINE : 0);

    m = calloc(1, sizeof *m);
    m->b = b;
    m->on = o;
    m->identity = copy(id, strlen(id));
    colon = strchr(id, ':');
    m->ns = copy(id, colon ? (size_t)(colon - id) : strlen(id));
    m->fuel = cfg->fuel;
    m->read = copy_list(cfg->read, cfg->read_count);
    m->read_count = cfg->read_count;
    m->write = copy_list(cfg->write, cfg->write_count);
    m->write_count = cfg->write_count;

    /* from here a failure is the module's, and its on_error decides what it costs */
    m->image = o->api.module_compile(o->be, a->bytes, a->len, a->path, demands, &err);
    if (!m->image) goto failed;
    if (private_memory(b, m, cfg, &err)) goto failed;
    store_header(b, m);

    /* a module is handed the memories it imports, and no other - backend.md */
    import_count = o->api.module_imports(m->image, imports, 64);
    for (i = 0; i < import_count && i < 64; i ++) {
        const backend_import *im = &imports[i];
        if (im->kind != BACKEND_IMPORT_MEMORY || im->ns_len != strlen(BRIDGE_NAMESPACE)
            || memcmp(im->ns, BRIDGE_NAMESPACE, im->ns_len) || memory_count == 2) continue;
        if (im->field_len == strlen(BRIDGE_SHARED) && !memcmp(im->field, BRIDGE_SHARED, im->field_len)) {
            memories[memory_count ++] = (backend_memory_import){ BRIDGE_NAMESPACE, BRIDGE_SHARED, o->shared };
        } else if (im->field_len == strlen(BRIDGE_PRIVATE)
                   && !memcmp(im->field, BRIDGE_PRIVATE, im->field_len)) {
            memories[memory_count ++] = (backend_memory_import){ BRIDGE_NAMESPACE, BRIDGE_PRIVATE, m->memory };
        }
    }
    m->instance = o->api.instance_create(o->be, m->image, memories, memory_count, &err);
    if (!m->instance) goto failed;
    if (abi_define(m, &err)) goto failed;
    if (cfg->memory_limit > 0) {
        o->api.instance_set_memory_limit(m->instance, (uint64_t)cfg->memory_limit);
    }

    init = o->api.instance_func(m->instance, "init", &arity, &results, &err);
    if (!init) goto failed;
    /* -1 is a backend that cannot tell, and then there is nothing to check - signatures.md */
    if ((arity != -1 && arity != 0) || (results != -1 && results != 1)) {
        snprintf(err.text, sizeof err.text, "its export 'init' is not () -> i32");
        goto failed;
    }
    if (cfg->init_timeout > 0) {
        o->api.instance_set_deadline(m->instance, (uint64_t)cfg->init_timeout);
    }
    m->in_init = 1;
    i = (uint32_t)o->api.call(init, &status, &err);
    m->in_init = 0;
    if (m->refused) {
        /* an entry refused, and Bridge-ASM700 knows why better than the trap does */
        snprintf(err.text, sizeof err.text, "%s", m->why);
        goto failed;
    }
    if (i) {
        char reason[sizeof err.text];
        snprintf(reason, sizeof reason, "%s", err.text);
        snprintf(err.text, sizeof err.text, "its init %.200s", reason);
        goto failed;
    }
    if (status) {
        snprintf(err.text, sizeof err.text, "its init answered %d", status);
        goto failed;
    }

    /* a system is an export, found once and checked here rather than on its first frame */
    for (i = 0; i < m->system_count; i ++) {
        bridge_system *s = m->systems[i];
        s->fn = o->api.instance_func(m->instance, s->name, &arity, &results, &err);
        if (!s->fn) goto failed;
        if ((arity != -1 && arity != 0) || (results != -1 && results != 1)) {
            snprintf(err.text, sizeof err.text, "its export '%.100s' is not () -> i32", s->name);
            goto failed;
        }
    }

    m->next = b->modules;
    b->modules = m;
    b->module_count ++;
    b->layout_moved = 1;
    store_barrier(b);           /* what init queued is applied, and its spans are published */
    return 0;

failed:
    say(b, NULL, NULL, BRIDGE_ERR, "module '%s' did not load: %s", id, err.text);
    if (cfg->on_error == BRIDGE_FAIL) {
        char why[256];
        snprintf(why, sizeof why, "module '%s' failed and its on_error is fail", id);
        b->stopped_by = copy(why, strlen(why));
    }
    /* ponytail: what its init declared and queued before failing stays. Leaving storage as
     * it was is docs/PLAN.md phase 6's claim, and arrives with unload. */
    discard(b, m);
    return 1;
}

int bridge_phase(bridge *b, const char *name)
{
    uint32_t phase, i, ran = 0;
    if (stopped(b)) return 1;
    for (phase = 0; phase < b->phase_count; phase ++) {
        if (name && !strcmp(b->phases[phase], name)) break;
    }
    if (phase == b->phase_count) {
        say(b, NULL, NULL, BRIDGE_ERR, "phase '%s' is not one the deployment declared",
            name ? name : "");
        return 1;
    }

    /* ponytail: registration order, one at a time. The conflict graph, [order] and threads
     * are docs/PLAN.md phase 5. */
    for (i = 0; i < b->system_count; i ++) {
        bridge_system *s = b->systems[i];
        module *m = s->m;
        bridge_header *h = (bridge_header *)m->priv;
        backend_err err = {0};
        int32_t status = 0;
        uint32_t slot = 0;
        if (s->phase != phase || m->stopped) continue;

        while (m->systems[slot] != s) slot ++;
        /* validation.md: the culprit is the system that was running. One store per dispatch. */
        h->when = b->barriers << 16 | (ran ++ & 0xffffu);
        h->system = slot;
        /* fuel only depletes, so it is set again before every dispatch - limits.md */
        if (m->fuel > 0) m->on->api.instance_set_fuel(m->instance, (uint64_t)m->fuel);
        m->refused = 0;
        if (m->on->api.call(s->fn, &status, &err) || m->refused) {
            /* failure.md: the invocation is abandoned, and logged */
            say(b, m, s, BRIDGE_ERR, "system '%s' failed: %s", s->name,
                m->refused ? m->why : err.text);
        } else if (status) {
            say(b, m, s, BRIDGE_ERR, "system '%s' failed: it answered %d", s->name, status);
        }
        h->system = UINT32_MAX;
    }
    store_barrier(b);
    return 0;
}
