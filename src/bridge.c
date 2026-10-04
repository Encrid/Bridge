/* Bridge-ASM700, as far as phase 2: open the backends, load a module through one, and say
 * why when it will not. docs/seam/host-api/create.md and lifecycle.md. */
#include "bridge.h"
#include "bridge-abi.h"
#include "backend.h"
#include "os.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char        *name;
    void        *lib;
    backend_api  api;
    backend     *be;
    uint32_t     capabilities;
} open_backend;

typedef struct loaded {
    open_backend     *on;
    backend_module   *module;
    backend_instance *instance;
    struct loaded    *next;
} loaded;

struct bridge {
    bridge_sink_fn sink;
    void          *ud;
    open_backend  *backends;
    uint32_t       backend_count;
    /* the address space every block is allocated with: the largest any backend stated */
    uint64_t       reservation, guard;
    loaded        *modules;
    char          *stopped_by;      /* a module whose failure was stated as fatal */
};

/* Bridge-ASM700's own records belong to no module: the text names whoever it is about. */
static void say(bridge *b, int8_t level, const char *fmt, ...)
{
    char text[512];
    bridge_record r = {0};
    va_list args;
    va_start(args, fmt);
    vsnprintf(text, sizeof text, fmt, args);
    va_end(args);
    r.text = text;
    r.len = (uint32_t)strlen(text);
    r.level = level;
    b->sink(b->ud, &r);
}

static char *copy(const char *s)
{
    size_t n = strlen(s) + 1;
    char *c = malloc(n);
    memcpy(c, s, n);
    return c;
}

void bridge_destroy(bridge *b)
{
    uint32_t i;
    if (!b) return;
    while (b->modules) {
        loaded *m = b->modules;
        b->modules = m->next;
        m->on->api.instance_destroy(m->instance);
        m->on->api.module_destroy(m->module);
        free(m);
    }
    for (i = 0; i < b->backend_count; i ++) {
        open_backend *o = &b->backends[i];
        if (o->be) o->api.backend_destroy(o->be);
        os_lib_close(o->lib);
        free(o->name);
    }
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

    for (i = 0; i < cfg->backend_count; i ++) {
        const bridge_backend *want = &cfg->backends[i];
        open_backend *o = &b->backends[i];
        char why[400];
        backend_err err = {0};
        uint64_t reservation = 0, guard = 0;

        if (!want->name || !want->library) {
            say(b, BRIDGE_ERR, "backend %u has no %s", i, want->name ? "library" : "name");
            goto refuse;
        }
        for (k = 0; k < i; k ++) {
            if (!strcmp(b->backends[k].name, want->name)) {
                say(b, BRIDGE_ERR, "backend '%s' is named twice", want->name);
                goto refuse;
            }
        }
        o->name = copy(want->name);
        b->backend_count = i + 1;
        o->lib = backend_open(want->library, &o->api, why, sizeof why);
        if (!o->lib) {
            say(b, BRIDGE_ERR, "backend '%s': %s", want->name, why);
            goto refuse;
        }
        o->be = o->api.backend_create(NULL, 0, &err);
        if (!o->be) {
            say(b, BRIDGE_ERR, "backend '%s' would not start: %s", want->name, err.text);
            goto refuse;
        }
        o->capabilities = o->api.backend_capabilities(o->be);
        o->api.memory_shape(o->be, &reservation, &guard);
        if (reservation > b->reservation) b->reservation = reservation;
        if (guard > b->guard) b->guard = guard;
    }
    return b;

refuse:
    bridge_destroy(b);
    return NULL;
}

/* A key whose zero means nobody stated it - config.md, *No unaccountable values*. */
static int unset(bridge *b, const char *identity, const char *key, int64_t value)
{
    if (value) return 0;
    say(b, BRIDGE_ERR, "module '%s' is refused: %s is unset", identity, key);
    return 1;
}

/* A stated limit demands that the backend can enforce it - capabilities.md. */
static int lacks(bridge *b, const char *identity, open_backend *o, uint32_t flag,
                 const char *key, const char *capability)
{
    if (o->capabilities & flag) return 0;
    say(b, BRIDGE_ERR, "module '%s' is refused: %s needs '%s', which backend '%s' lacks",
        identity, key, capability, o->name);
    return 1;
}

int bridge_load(bridge *b, const bridge_artifact *a)
{
    const bridge_module_config *cfg = &a->cfg;
    const char *id = cfg->identity;
    open_backend *o = NULL;
    backend_module *module = NULL;
    backend_instance *instance = NULL;
    backend_func *init;
    backend_err err = {0};
    int32_t arity = 0, results = 0, status = 0;
    loaded *m;
    uint32_t i;

    if (b->stopped_by) {
        say(b, BRIDGE_ERR, "the bridge has stopped: module '%s' failed and its on_error is fail",
            b->stopped_by);
        return 1;
    }
    if (!id || !*id) {
        say(b, BRIDGE_ERR, "a module with no identity is refused");
        return 1;
    }

    /* everything that can be decided from cfg alone is decided before the artifact is touched */
    for (i = 0; i < b->backend_count && a->backend; i ++) {
        if (!strcmp(b->backends[i].name, a->backend)) o = &b->backends[i];
    }
    if (!o) {
        say(b, BRIDGE_ERR, "module '%s' is refused: no backend named '%s'", id,
            a->backend ? a->backend : "");
        return 1;
    }
    if (cfg->abi != BRIDGE_ABI) {
        say(b, BRIDGE_ERR, "module '%s' is refused: it wants ABI %u and this is ABI %u", id,
            cfg->abi, (unsigned)BRIDGE_ABI);
        return 1;
    }
    if (cfg->on_error != BRIDGE_FAIL && cfg->on_error != BRIDGE_SKIP) {
        say(b, BRIDGE_ERR, "module '%s' is refused: on_error is unset", id);
        return 1;
    }
    if (unset(b, id, "memory_limit", cfg->memory_limit)
        || unset(b, id, "init_timeout", cfg->init_timeout)) return 1;
    if ((!cfg->trusted && lacks(b, id, o, BACKEND_SANDBOX, "trusted = false", "sandbox"))
        || (cfg->memory_limit > 0 && lacks(b, id, o, BACKEND_LIMIT, "memory_limit", "limit"))
        || (cfg->init_timeout > 0 && lacks(b, id, o, BACKEND_DEADLINE, "init_timeout", "deadline"))) {
        return 1;
    }

    /* from here a failure is the module's, and its on_error decides what it costs */
    module = o->api.module_compile(o->be, a->bytes, a->len, a->path, &err);
    if (!module) goto failed;
    instance = o->api.instance_create(o->be, module, NULL, 0, &err);
    if (!instance) goto failed;
    if (cfg->memory_limit > 0) {
        o->api.instance_set_memory_limit(instance, (uint64_t)cfg->memory_limit);
    }

    init = o->api.instance_func(instance, "init", &arity, &results, &err);
    if (!init) goto failed;
    /* -1 is a backend that cannot tell, and then there is nothing to check - signatures.md */
    if ((arity != -1 && arity != 0) || (results != -1 && results != 1)) {
        snprintf(err.text, sizeof err.text, "its export 'init' is not () -> i32");
        goto failed;
    }
    if (cfg->init_timeout > 0) {
        o->api.instance_set_deadline(instance, (uint64_t)cfg->init_timeout);
    }
    if (o->api.call(init, &status, &err)) {
        char reason[sizeof err.text];
        snprintf(reason, sizeof reason, "%s", err.text);
        snprintf(err.text, sizeof err.text, "its init %.200s", reason);
        goto failed;
    }
    if (status) {
        snprintf(err.text, sizeof err.text, "its init answered %d", status);
        goto failed;
    }

    m = calloc(1, sizeof *m);
    m->on = o;
    m->module = module;
    m->instance = instance;
    m->next = b->modules;
    b->modules = m;
    return 0;

failed:
    say(b, BRIDGE_ERR, "module '%s' did not load: %s", id, err.text);
    if (instance) o->api.instance_destroy(instance);
    if (module) o->api.module_destroy(module);
    if (cfg->on_error == BRIDGE_FAIL) b->stopped_by = copy(id);
    return 1;
}
