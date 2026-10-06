/* Bridge-ASM700: open the backends, build storage, run a phase, and say what is loaded. Loading,
 * unloading and reloading a module are lifecycle.c.
 * docs/seam/host-api/create.md, loop.md and introspection.md. */
#include "inside.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* A record of Bridge-ASM700's own. `m` and `s` say whose it is about, when it is anybody's. */
void say(bridge *b, const module *m, const bridge_system *s, uint8_t level, const char *fmt, ...)
{
    char text[512];
    bridge_record r = {};
    va_list args;
    va_start(args, fmt);
    vsnprintf(text, sizeof text, fmt, args);
    va_end(args);
    r.identity = m ? m->identity : nullptr;
    r.system = s ? s->name : nullptr;
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

int bridge_destroy(bridge *b)
{
    uint32_t i;
    module *m;
    if (!b) return 0;
    /* pinned.md: a pinned module's shutdown is its code, and runs on its thread or not at
     * all. Refused before anything is touched, so the bridge is whole for the call that can. */
    for (m = b->modules; m; m = m->next) {
        if (m->pinned && m->thread != os_thread_id()) {
            say(b, nullptr, nullptr, BRIDGE_ERR, "bridge_destroy is refused: module '%s' is pinned to "
                "the thread that loaded it, and this call comes from another", m->identity);
            return 1;
        }
    }
    /* each module's shutdown, in the order they loaded: nothing here depends on anything, so
     * no other order would mean more */
    for (m = b->modules; m; m = m->next) module_shut_down(b, m);
    while (b->modules) {
        m = b->modules;
        b->modules = m->next;
        module_discard(b, m);
    }
    free(b->systems);
    free(b->run_jobs);
    free(b->run_handed);
    free(b->run_lists);
    free(b->run_callers);
    block_destroy(b);
    pool_destroy(b->pool);
    for (i = 0; i < b->order_count; i ++) {
        free(b->before[i]);
        free(b->after[i]);
    }
    free(b->before);
    free(b->after);
    for (i = 0; i < b->backend_count; i ++) {
        open_backend *o = &b->backends[i];
        if (o->be) o->api.backend_destroy(o->be);
        /* capabilities.md: only a backend that says its library can go. One that brought a
         * runtime in is left in the process, where the next bridge finds it again. A library
         * that was opened and never became a backend said nothing, and is left too. */
        if (o->lib && (o->capabilities & BACKEND_UNLOAD)) os_lib_close(o->lib);
        free(o->name);
    }
    for (i = 0; i < b->phase_count; i ++) free(b->phases[i]);
    free(b->phases);
    free(b->backends);
    free(b->stopped_by);
    free(b->stopped_stem);
    free(b);
    return 0;
}

/* the least length that is a whole number of both */
static uint64_t common(uint64_t a, uint64_t b)
{
    uint64_t x = a, y = b;
    while (y) { uint64_t r = x % y; x = y; y = r; }
    return a / x * b;
}

bridge *bridge_create(const bridge_config *cfg, bridge_sink_fn sink, void *ud)
{
    bridge *b;
    uint32_t i, k;
    /* the one failure no record can explain, and a host that passed null already knows */
    if (!sink || !cfg) return nullptr;

    b = calloc(1, sizeof *b);
    b->granule = 1;
    b->sink = sink;
    b->ud = ud;
    /* host-api.md, *A seam has a number*: read before anything else of the struct is */
    if (cfg->abi != BRIDGE_HOST_ABI) {
        if (!cfg->abi) say(b, nullptr, nullptr, BRIDGE_ERR, "abi is unset");
        else say(b, nullptr, nullptr, BRIDGE_ERR, "the host is of host ABI %u, and this is %u",
            cfg->abi, (unsigned)BRIDGE_HOST_ABI);
        goto refuse;
    }
    b->backends = calloc(cfg->backend_count ? cfg->backend_count : 1, sizeof *b->backends);
    b->phases = calloc(cfg->phase_count ? cfg->phase_count : 1, sizeof *b->phases);

    for (i = 0; i < cfg->backend_count; i ++) {
        const bridge_backend *want = &cfg->backends[i];
        open_backend *o = &b->backends[i];
        char why[400];
        backend_err err = {};
        uint64_t reservation = 0, guard = 0, granule = 0;

        if (!want->name || !want->library) {
            say(b, nullptr, nullptr, BRIDGE_ERR, "backend %u has no %s", i,
                want->name ? "library" : "name");
            goto refuse;
        }
        for (k = 0; k < i; k ++) {
            if (!strcmp(b->backends[k].name, want->name)) {
                say(b, nullptr, nullptr, BRIDGE_ERR, "backend '%s' is named twice", want->name);
                goto refuse;
            }
        }
        o->name = copy(want->name, strlen(want->name));
        b->backend_count = i + 1;
        o->lib = backend_open(want->library, &o->api, why, sizeof why);
        if (!o->lib) {
            say(b, nullptr, nullptr, BRIDGE_ERR, "backend '%s': %s", want->name, why);
            goto refuse;
        }
        {
            /* two structs for one fact, one per seam, so that neither header needs the
             * other: held to one shape here, where one is copied to the other */
            static_assert((int)BRIDGE_BOOL == (int)BACKEND_BOOL && (int)BRIDGE_INT == (int)BACKEND_INT
                && (int)BRIDGE_FLOAT == (int)BACKEND_FLOAT && (int)BRIDGE_STRING == (int)BACKEND_STRING);
            static_assert(sizeof(bridge_setting) == sizeof(backend_setting)
                && offsetof(bridge_setting, kind) == offsetof(backend_setting, kind)
                && offsetof(bridge_setting, v) == offsetof(backend_setting, v)
                && sizeof(((bridge_setting *)nullptr)->v.b) == sizeof(((backend_setting *)nullptr)->v.b));
            backend_setting *settings = calloc(want->setting_count ? want->setting_count : 1,
                                               sizeof *settings);
            for (k = 0; k < want->setting_count; k ++) {
                const bridge_setting *from = &want->settings[k];
                settings[k].key = from->key;
                /* the same four kinds under the two seams' names, and the same numbers */
                settings[k].kind = (enum backend_setting_kind)from->kind;
                if (from->kind == BRIDGE_BOOL) settings[k].v.b = from->v.b;
                else if (from->kind == BRIDGE_INT) settings[k].v.i = from->v.i;
                else if (from->kind == BRIDGE_FLOAT) settings[k].v.f = from->v.f;
                else settings[k].v.s = from->v.s;
            }
            o->be = o->api.backend_create(settings, want->setting_count, &err);
            free(settings);
        }
        if (!o->be) {
            say(b, nullptr, nullptr, BRIDGE_ERR, "backend '%s' would not start: %s", want->name, err.text);
            goto refuse;
        }
        o->capabilities = o->api.backend_capabilities(o->be);
        o->api.memory_shape(o->be, &reservation, &guard, &granule);
        o->reservation = reservation;
        o->guard = guard;
        o->granule = granule ? granule : 1;
        if (reservation > b->reservation) b->reservation = reservation;
        if (guard > b->guard) b->guard = guard;
        /* every backend maps the shared block, so its length is a multiple of each one's */
        b->granule = common(b->granule, o->granule);
    }

    /* schedule.md, *Bridge-ASM700 ships no phase names*: a deployment declares its own */
    if (!cfg->phase_count) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "the deployment declares no phase, and there is none "
            "of Bridge-ASM700's to run");
        goto refuse;
    }
    for (i = 0; i < cfg->phase_count; i ++) {
        const char *name = cfg->phases[i];
        if (!name || !*name) {
            say(b, nullptr, nullptr, BRIDGE_ERR, "phase %u has no name", i);
            goto refuse;
        }
        for (k = 0; k < i; k ++) {
            if (!strcmp(b->phases[k], name)) {
                say(b, nullptr, nullptr, BRIDGE_ERR, "phase '%s' is named twice", name);
                goto refuse;
            }
        }
        b->phases[i] = copy(name, strlen(name));
        b->phase_count = i + 1;
    }

    if (schedule_order(b, cfg->order, cfg->order_count)) goto refuse;
    /* config.md, *No unaccountable values*: how many threads is not ours to guess */
    if (!cfg->threads) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "threads is unset");
        goto refuse;
    }
    /* a job is a module's systems of one level, so there are never more than systems */
    b->pool = pool_create(cfg->threads - 1, cfg->budget.systems);

    if (block_create(b, &cfg->budget)) goto refuse;
    schedule_room(b);
    return b;

refuse:
    (void)bridge_destroy(b);        /* nothing is loaded, so nothing can refuse it */
    return nullptr;
}

static size_t stem_length(const char *identity)
{
    const char *at = strchr(identity, '@');
    return at ? (size_t)(at - identity) : strlen(identity);
}

/* failure.md: a module stated `fail` is lost, and nothing runs until it is back. */
void bridge_stop(bridge *b, const char *identity, const char *what)
{
    char why[256];
    if (b->stopped_by) return;
    snprintf(why, sizeof why, "module '%s' %s and its on_error is fail", identity, what);
    b->stopped_by = copy(why, strlen(why));
    b->stopped_stem = copy(identity, stem_length(identity));
    say(b, nullptr, nullptr, BRIDGE_ERR, "the bridge has stopped: %s", why);
}

void bridge_resume(bridge *b, const module *m)
{
    if (!b->stopped_stem || strcmp(b->stopped_stem, m->stem)) return;
    free(b->stopped_by);
    free(b->stopped_stem);
    b->stopped_by = b->stopped_stem = nullptr;
    say(b, m, nullptr, 0, "the bridge runs again: module '%s' is back", m->identity);
}

/* failure.md: the host unloads the module that stopped the bridge, by name, and so says the
 * deployment goes on without it. */
void bridge_given_up(bridge *b, const module *m)
{
    if (!b->stopped_stem || strcmp(b->stopped_stem, m->stem)) return;
    free(b->stopped_by);
    free(b->stopped_stem);
    b->stopped_by = b->stopped_stem = nullptr;
    say(b, m, nullptr, 0, "the bridge runs again: module '%s' is unloaded, and it goes on without it",
        m->identity);
}

int bridge_stopped(bridge *b, const char *repairing)
{
    module *m;
    if (b->stopped_by) {
        /* what stopped it can be brought back, or given up by an unload of it, and those
         * are the calls it accepts */
        size_t n = repairing ? stem_length(repairing) : 0;
        if (!repairing || !b->stopped_stem || strlen(b->stopped_stem) != n
            || memcmp(b->stopped_stem, repairing, n)) {
            say(b, nullptr, nullptr, BRIDGE_ERR, "the bridge has stopped: %s", b->stopped_by);
            return 1;
        }
    }
    /* pinned.md: a pinned module's thread is the one that loaded it, and Bridge-ASM700 has no
     * other to run it on. Both calls that run module code come through here. */
    for (m = b->modules; m; m = m->next) {
        if (m->pinned && m->thread != os_thread_id()) {
            say(b, nullptr, nullptr, BRIDGE_ERR, "refused: module '%s' is pinned to the thread that "
                "loaded it, and this call comes from another", m->identity);
            return 1;
        }
    }
    return 0;
}

int bridge_phase(bridge *b, const char *name)
{
    uint32_t phase;
    if (bridge_stopped(b, nullptr)) return 1;
    for (phase = 0; phase < b->phase_count; phase ++) {
        if (name && !strcmp(b->phases[phase], name)) break;
    }
    if (phase == b->phase_count) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "phase '%s' is not one the deployment declared",
            name ? name : "");
        return 1;
    }

    schedule_run(b, phase);
    block_barrier(b);
    return 0;
}

/* introspection.md: a pull, because a stop must not arrive by a route that may drop it */
int bridge_modules(bridge *b, bridge_module_info *out, int max)
{
    module *m;
    int count = 0;
    for (m = b->modules; m; m = m->next) {
        if (out && count < max) {
            out[count].identity = m->identity;
            out[count].state = m->stopped ? BRIDGE_STOPPED : BRIDGE_RUNNING;
        }
        count ++;
    }
    return count;
}
