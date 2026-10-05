/* The runtime lifecycle: load, unload, reload. docs/seam/host-api/lifecycle.md is the order of
 * each; docs/bridge/data/lifetime.md is what becomes of storage; docs/bridge/run/failure.md is
 * what a failure costs. All three are legal only between phases. */
#include "inside.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

/* ---- components leaving ---- */

/* A component's record goes: its tables' ids, its flecs entity, and its hold on the types its
 * fields name, which go with it when nothing else declares or names them. */
static void component_delete(bridge *b, component *c)
{
    uint32_t k;
    store_retire(b, c->id);             /* before flecs deletes the tables that hold it */
    ecs_delete(b->world, c->id);
    for (k = 0; b->components[k] != c; k ++) {}
    b->components[k] = b->components[-- b->component_count];
    for (k = 0; k < c->dep_count; k ++) {
        component *named = c->deps[k];
        if (!-- named->users && !named->declarers) component_delete(b, named);
    }
    free(c->deps);
    free(c->name);
    free(c->zero);
    free(c);
}

/* lifetime.md, *What unload leaves behind*: what becomes of the entities that carried it */
static void drop_rows(bridge *b, component *c, uint32_t policy)
{
    ecs_entity_t *carried = NULL;
    uint32_t count = 0, room = 0, i;
    if (policy == BRIDGE_DELETE) {
        ecs_delete_with(b->world, c->id);
        return;
    }
    if (policy == BRIDGE_SWEEP) {
        ecs_iter_t it = ecs_each_id(b->world, c->id);
        while (ecs_each_next(&it)) {
            int32_t r;
            for (r = 0; r < it.count; r ++) {
                if (count == room) {
                    room = room ? room * 2 : 64;
                    carried = realloc(carried, room * sizeof *carried);
                }
                carried[count ++] = it.entities[r];
            }
        }
    }
    ecs_remove_all(b->world, c->id);
    /* sweep: an entity left with nothing could never be found again, so it goes too */
    for (i = 0; i < count; i ++) {
        const ecs_type_t *type = ecs_is_alive(b->world, carried[i])
                               ? ecs_get_type(b->world, carried[i]) : NULL;
        if (type && !type->count) ecs_delete(b->world, carried[i]);
    }
    free(carried);
}

/* A module stops declaring `c`. Shared, it stays; the last to leave applies its own policy.
 * Policy zero is a load that failed: nothing can carry what was never applied. */
static void component_release(bridge *b, component *c, uint32_t policy)
{
    if (-- c->declarers) return;
    if (policy) drop_rows(b, c, policy);
    /* a type some other component's field still names is kept as a type, with no rows */
    if (!c->users) component_delete(b, c);
    b->layout_moved = 1;
}

/* ---- a module's record ---- */

static void drop_systems(bridge *b, module *m)
{
    uint32_t i, kept = 0;
    for (i = 0; i < b->system_count; i ++) {
        bridge_system *s = b->systems[i];
        if (s->m != m) {
            b->systems[kept ++] = s;
            continue;
        }
        if (s->query) ecs_query_fini(s->query);
        if (s->observing) ecs_delete(b->world, s->observing);
        free(s->pending);
        free(s->name);
        free(s->full);
        free(s);
    }
    b->system_count = kept;
    m->system_count = m->observer_count = 0;
}

/* Its systems, its instance, its image, its private memory, and the record. The order is the
 * backend seam's: a memory is released after the last instance it was handed to. */
void module_discard(bridge *b, module *m)
{
    drop_systems(b, m);
    if (m->instance) m->on->api.instance_destroy(m->instance);
    if (m->image) m->on->api.module_destroy(m->image);
    if (m->memory) m->on->api.memory_release(m->memory);
    if (m->priv) os_release(m->priv, m->priv_reserve);
    free_list_of(m->read, m->read_count);
    free_list_of(m->write, m->write_count);
    free(m->ids);
    free(m->queue);
    free(m->systems);
    free(m->observers);
    free(m->declared);
    free(m->policy);
    free(m->identity);
    free(m->stem);
    free(m->ns);
    free(m);
}

/* A load that failed leaves storage as it was: the ids set aside for it, the row its settings
 * were on, what it took of the arena, and every component nobody else declared. What it
 * queued goes with its queue. */
static void undo(bridge *b, module *m)
{
    uint32_t i;
    drop_systems(b, m);         /* first: a query must not outlive a component it names */
    for (i = 0; i < m->id_count; i ++) ecs_delete(b->world, m->ids[i]);
    m->id_count = m->id_next = 0;
    if (m->config_entity) ecs_delete(b->world, m->config_entity);
    /* its share is the last one carved, since nothing loads during a load */
    if (m->arena_share && m->arena_base + m->arena_share == b->arena + b->arena_used) {
        b->arena_used -= m->arena_share;
    }
    /* last declared first: a type is used by what was declared after it */
    for (i = m->declared_count; i --;) component_release(b, m->declared[i], 0);
    m->declared_count = 0;
    b->layout_moved = 1;
}

/* ---- what can be decided from a config alone ---- */

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

/* Everything a load and a reload both refuse before the artifact is touched. */
static int checks(bridge *b, const bridge_artifact *a, open_backend **on, uint32_t *demands)
{
    const bridge_module_config *cfg = &a->cfg;
    const char *id = cfg->identity;
    open_backend *o = NULL;
    uint32_t i;

    if (!id || !*id) {
        say(b, NULL, NULL, BRIDGE_ERR, "a module with no identity is refused");
        return 1;
    }
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
    if (unset(b, id, "max_system_failures", cfg->max_system_failures)
        || unset(b, id, "fuel", cfg->fuel) || unset(b, id, "memory_limit", cfg->memory_limit)
        || unset(b, id, "init_timeout", cfg->init_timeout)
        || unset(b, id, "reload_timeout", cfg->reload_timeout)
        || unset(b, id, "shutdown_timeout", cfg->shutdown_timeout)
        || unset(b, id, "scratch", cfg->scratch) || unset(b, id, "log_ring", cfg->log_ring)
        || unset(b, id, "spans", cfg->spans) || unset(b, id, "spawns", cfg->spawns)
        || unset(b, id, "persistent", cfg->persistent)) {
        return 1;
    }
    if (cfg->spawns > (int64_t)b->entities) {
        say(b, NULL, NULL, BRIDGE_ERR, "module '%s' is refused: its spawns of %lld is more "
            "than [budget] entities of %u", id, (long long)cfg->spawns, b->entities);
        return 1;
    }
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
        || (cfg->init_timeout > 0 && lacks(b, id, o, BACKEND_DEADLINE, "init_timeout", "deadline"))
        || (cfg->reload_timeout > 0 && lacks(b, id, o, BACKEND_DEADLINE, "reload_timeout", "deadline"))
        || (cfg->shutdown_timeout > 0
            && lacks(b, id, o, BACKEND_DEADLINE, "shutdown_timeout", "deadline"))
        || (cfg->hot_reload && lacks(b, id, o, BACKEND_RELOAD, "hot_reload", "reload"))) {
        return 1;
    }
    /* wasm.md, *Exports*: the author says "cannot" and the deployment "may not", each with a
     * key. A deployment allowing what the author says cannot be done is config against config. */
    if (cfg->hot_reload && !cfg->reloadable) {
        say(b, NULL, NULL, BRIDGE_ERR, "module '%s' is refused: hot_reload is stated, and its "
            "author states it is not reloadable", id);
        return 1;
    }
    *on = o;
    *demands = (cfg->trusted ? 0 : BACKEND_SANDBOX) | (cfg->fuel > 0 ? BACKEND_METER : 0)
             | (cfg->memory_limit > 0 ? BACKEND_LIMIT : 0)
             | (cfg->init_timeout > 0 || cfg->reload_timeout > 0 || cfg->shutdown_timeout > 0
                ? BACKEND_DEADLINE : 0);
    return 0;
}

static size_t stem_of(const char *identity)
{
    const char *at = strchr(identity, '@');
    return at ? (size_t)(at - identity) : strlen(identity);
}

/* a system is named by its module without the version, so that is what tells modules apart -
 * schedule.md, *Order within a phase is never implicit* */
static module *loaded(bridge *b, const char *identity)
{
    size_t stem = stem_of(identity);
    module *m;
    for (m = b->modules; m; m = m->next) {
        if (strlen(m->stem) == stem && !memcmp(m->stem, identity, stem)) return m;
    }
    return NULL;
}

static module *record(bridge *b, const bridge_artifact *a, open_backend *o)
{
    const bridge_module_config *cfg = &a->cfg;
    const char *id = cfg->identity, *colon = strchr(id, ':');
    module *m = calloc(1, sizeof *m);
    m->b = b;
    m->on = o;
    m->identity = copy(id, strlen(id));
    m->stem = copy(id, stem_of(id));
    m->ns = copy(id, colon ? (size_t)(colon - id) : strlen(id));
    m->pinned = cfg->pinned;
    m->thread = os_thread_id();
    m->on_error = cfg->on_error;
    m->hot_reload = cfg->hot_reload;
    m->max_failures = cfg->max_system_failures;
    m->shutdown_timeout = cfg->shutdown_timeout;
    m->fuel = cfg->fuel;
    m->spawns = cfg->spawns;
    m->persistent = cfg->persistent;
    m->read = copy_list(cfg->read, cfg->read_count);
    m->read_count = cfg->read_count;
    m->write = copy_list(cfg->write, cfg->write_count);
    m->write_count = cfg->write_count;
    return m;
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

/* An export Bridge-ASM700 calls: found once, and checked to be () -> i32 where the backend
 * can tell - signatures.md. */
static backend_func *export_of(module *m, const char *name, backend_err *err)
{
    int32_t arity = 0, results = 0;
    backend_func *f = m->on->api.instance_func(m->instance, name, &arity, &results, err);
    if (!f) return NULL;
    if ((arity != -1 && arity != 0) || (results != -1 && results != 1)) {
        snprintf(err->text, sizeof err->text, "its export '%.100s' is not () -> i32", name);
        return NULL;
    }
    return f;
}

/* One call of a lifecycle export, under its timeout. 0 if it returned 0. */
static int lifecycle_call(module *m, backend_func *f, const char *name, int64_t timeout,
                          backend_err *err)
{
    int32_t status = 0;
    int trapped;
    if (timeout > 0) m->on->api.instance_set_deadline(m->instance, (uint64_t)timeout);
    m->refused = 0;
    trapped = m->on->api.call(f, &status, err);
    if (m->refused) {
        /* an entry refused, and Bridge-ASM700 knows why better than the trap does */
        snprintf(err->text, sizeof err->text, "%s", m->why);
        return 1;
    }
    if (trapped) {
        char reason[sizeof err->text];
        snprintf(reason, sizeof reason, "%s", err->text);
        snprintf(err->text, sizeof err->text, "its %s %.200s", name, reason);
        return 1;
    }
    if (status) {
        snprintf(err->text, sizeof err->text, "its %s answered %d", name, status);
        return 1;
    }
    return 0;
}

/* An image, an instance, a private memory, and the first call into it: `init` for a load,
 * `reload` for a reload. 0 if the module registered and every export it named is there. */
static int start(bridge *b, module *m, const bridge_artifact *a, uint32_t demands,
                 const char *entry, backend_err *err)
{
    const bridge_module_config *cfg = &a->cfg;
    open_backend *o = m->on;
    backend_import imports[64];
    backend_memory_import memories[2];
    uint32_t i, import_count, memory_count = 0;
    backend_func *first;
    int reloading = !strcmp(entry, "reload");

    m->image = o->api.module_compile(o->be, a->bytes, a->len, a->path, demands, err);
    if (!m->image) return 1;
    if (private_memory(b, m, cfg, err)) return 1;
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
    m->instance = o->api.instance_create(o->be, m->image, memories, memory_count, err);
    if (!m->instance) return 1;
    if (abi_define(m, err)) return 1;
    if (cfg->memory_limit > 0) {
        o->api.instance_set_memory_limit(m->instance, (uint64_t)cfg->memory_limit);
    }

    first = export_of(m, entry, err);
    if (!first) return 1;
    m->in_init = 1;
    i = (uint32_t)lifecycle_call(m, first, entry, reloading ? cfg->reload_timeout : cfg->init_timeout, err);
    m->in_init = 0;
    if (i) return 1;

    /* config.md: one row, written after init has declared the struct it is a row of */
    if (!reloading && config_seed(b, m, cfg, err->text, sizeof err->text)) return 1;

    /* a system is an export, found once and checked here rather than on its first frame */
    for (i = 0; i < m->system_count + m->observer_count; i ++) {
        bridge_system *s = i < m->system_count ? m->systems[i] : m->observers[i - m->system_count];
        s->fn = export_of(m, s->name, err);
        if (!s->fn) return 1;
    }
    /* the three lifecycle exports are every module's: none is left out to mean something */
    m->shutdown = export_of(m, "shutdown", err);
    if (!m->shutdown || !export_of(m, "reload", err)) return 1;
    return 0;
}

static void stop_the_bridge(bridge *b, const char *identity, const char *what)
{
    char why[256];
    if (b->stopped_by) return;
    snprintf(why, sizeof why, "module '%s' %s and its on_error is fail", identity, what);
    b->stopped_by = copy(why, strlen(why));
}

/* ---- load ---- */

int bridge_load(bridge *b, const bridge_artifact *a)
{
    const bridge_module_config *cfg = &a->cfg;
    const char *id = cfg->identity;
    open_backend *o = NULL;
    backend_err err = {0};
    uint32_t demands = 0;
    module *m, *other;

    store_enter(b);
    if (bridge_stopped(b) || checks(b, a, &o, &demands)) return 1;
    other = loaded(b, id);
    if (other) {
        say(b, NULL, NULL, BRIDGE_ERR, "module '%s' is refused: '%s' is loaded already", id,
            other->identity);
        return 1;
    }
    if (b->module_count >= b->max_modules) {
        say(b, NULL, NULL, BRIDGE_ERR, "module '%s' is refused: [budget] max_modules of %u is "
            "reached", id, b->max_modules);
        return 1;
    }
    if (cfg->persistent > 0 && (uint64_t)cfg->persistent > b->arena_len - b->arena_used) {
        say(b, NULL, NULL, BRIDGE_ERR, "module '%s' is refused: its persistent of %lld bytes "
            "does not fit in the %u left of [budget] persistent", id, (long long)cfg->persistent,
            b->arena_len - b->arena_used);
        return 1;
    }

    m = record(b, a, o);
    if (cfg->persistent > 0) {
        /* its share of the arena, for its life: nobody else allocates from it */
        uint32_t share = ((uint32_t)cfg->persistent + 7u) & ~7u;
        if (share > b->arena_len - b->arena_used) share = b->arena_len - b->arena_used;
        m->arena_share = share;
        m->arena_base = m->arena_at = b->arena + b->arena_used;
        m->arena_left = share;
        b->arena_used += share;
    }

    /* from here a failure is the module's, and its on_error decides what it costs. init may
     * spawn, so its ids are set aside before it runs. */
    if (store_refill(b, m)) {
        snprintf(err.text, sizeof err.text, "its spawns of %lld do not fit under [budget] "
            "entities of %u", (long long)cfg->spawns, b->entities);
        goto failed;
    }
    if (start(b, m, a, demands, "init", &err)) goto failed;
    /* lifecycle.md: refuse-and-unwind if a new system or observer conflicts unordered */
    if (schedule_build(b, err.text, sizeof err.text)) goto failed;

    /* at the end: the order modules loaded in is the order their changes are applied in */
    {
        module **last = &b->modules;
        while (*last) last = &(*last)->next;
        *last = m;
    }
    b->module_count ++;
    b->layout_moved = 1;
    store_barrier(b);           /* what init queued is applied, and its spans are published */
    /* the module is in, and nothing will ever run it: that is not a load that succeeded */
    return b->stopped_by ? 1 : 0;

failed:
    say(b, NULL, NULL, BRIDGE_ERR, "module '%s' did not load: %s", id, err.text);
    if (cfg->on_error == BRIDGE_FAIL) stop_the_bridge(b, id, "failed");
    undo(b, m);
    module_discard(b, m);
    schedule_build(b, err.text, sizeof err.text);   /* as it was: this cannot refuse */
    return 1;
}

/* ---- unload ---- */

/* failure.md: a shutdown that fails is logged, and the teardown continues. Nothing else is
 * coherent, and it is not tried again. */
static void shut_down(bridge *b, module *m)
{
    backend_err err = {0};
    if (!m->instance || !m->shutdown) return;
    if (lifecycle_call(m, m->shutdown, "shutdown", m->shutdown_timeout, &err)) {
        say(b, m, NULL, BRIDGE_ERR, "module '%s': %s; its teardown continues", m->identity, err.text);
    }
    store_apply(b, m);          /* what it had asked for, shutdown's requests included */
    store_drain(b, m);          /* and what it said: its ring is about to go with it */
}

/* The rest of an unload, once the module's code has run for the last time. */
static void take_out(bridge *b, module *m)
{
    module **link = &b->modules;
    uint32_t i;
    drop_systems(b, m);
    if (m->config_entity) ecs_delete(b->world, m->config_entity);
    /* last declared first: a type is used by what was declared after it */
    for (i = m->declared_count; i --;) component_release(b, m->declared[i], m->policy[i]);
    m->declared_count = 0;
    /* the ids held ready for it are entities nobody will ever spawn */
    for (i = m->id_next; i < m->id_count; i ++) ecs_delete(b->world, m->ids[i]);
    config_keep(b, m);
    while (*link != m) link = &(*link)->next;
    *link = m->next;
    b->module_count --;
    module_discard(b, m);
}

int bridge_unload(bridge *b, const char *identity)
{
    char why[256];
    module *m;
    store_enter(b);
    if (bridge_stopped(b)) return 1;
    for (m = b->modules; m && (!identity || strcmp(m->identity, identity)); m = m->next) {}
    if (!m) {
        say(b, NULL, NULL, BRIDGE_ERR, "no module '%s' is loaded, so none is unloaded",
            identity ? identity : "");
        return 1;
    }
    drop_systems(b, m);         /* out of the schedule before its last call */
    shut_down(b, m);
    take_out(b, m);
    schedule_build(b, why, sizeof why);
    b->layout_moved = 1;
    store_barrier(b);           /* what left is out of every module's spans, and observed */
    return 0;
}

/* ---- reload ---- */

int bridge_reload(bridge *b, const bridge_artifact *a)
{
    const bridge_module_config *cfg = &a->cfg;
    const char *id = cfg->identity;
    open_backend *o = NULL;
    backend_err err = {0};
    uint32_t demands = 0, i;
    module *old, *n, **link;
    int side_by_side;

    store_enter(b);
    if (bridge_stopped(b) || checks(b, a, &o, &demands)) return 1;
    old = loaded(b, id);
    if (!old) {
        say(b, NULL, NULL, BRIDGE_ERR, "module '%s' is not reloaded: nothing of that name is "
            "loaded", id);
        return 1;
    }
    /* wasm.md, *Exports*: hot_reload only ever forbids */
    if (!old->hot_reload) {
        say(b, NULL, NULL, BRIDGE_ERR, "module '%s' is not reloaded: its hot_reload is false",
            old->identity);
        return 1;
    }
    if (o != old->on) {
        say(b, NULL, NULL, BRIDGE_ERR, "module '%s' is not reloaded: it runs on backend '%s' "
            "and a reload does not move it to '%s'", old->identity, old->on->name, o->name);
        return 1;
    }
    /* its share lasts as long as it is loaded, and others hold offsets into it */
    if (cfg->persistent != old->persistent) {
        say(b, NULL, NULL, BRIDGE_ERR, "module '%s' is not reloaded: its persistent is %lld "
            "and a reload states %lld, which only an unload and a load can change",
            old->identity, (long long)old->persistent, (long long)cfg->persistent);
        return 1;
    }

    /* The new instance takes over what was set aside for the module. Until the swap these
     * are copies: a reload that fails leaves the old record's own as they were. */
    n = record(b, a, o);
    n->ids = old->ids;
    n->id_count = old->id_count;
    n->id_next = old->id_next;
    n->arena_base = old->arena_base;
    n->arena_share = old->arena_share;
    n->arena_at = old->arena_at;
    n->arena_left = old->arena_left;
    n->config = old->config;
    n->config_entity = old->config_entity;
    n->text_at = old->text_at;
    n->text_room = old->text_room;

    /* the old instance's systems stand aside: the new one registers the same names */
    for (i = 0; i < b->system_count; i ++) {
        if (b->systems[i]->m == old) b->systems[i]->parked = 1;
    }
    side_by_side = (o->capabilities & BACKEND_SIDE_BY_SIDE) != 0;
    if (!side_by_side) {
        /* a process cannot hold two copies of one library: the old one goes first */
        shut_down(b, old);
        o->api.instance_destroy(old->instance);
        o->api.module_destroy(old->image);
        old->instance = NULL;
        old->image = NULL;
    }

    if (start(b, n, a, demands, "reload", &err) || schedule_build(b, err.text, sizeof err.text)) {
        say(b, NULL, NULL, BRIDGE_ERR, "module '%s' was not reloaded: %s", id, err.text);
        drop_systems(b, n);
        for (i = n->declared_count; i --;) component_release(b, n->declared[i], 0);
        n->declared_count = 0;
        n->ids = NULL;              /* they are the old record's */
        n->config_entity = 0;
        module_discard(b, n);
        if (side_by_side) {
            /* failure.md: the old instance is kept, and it was never touched */
            for (i = 0; i < b->system_count; i ++) b->systems[i]->parked = 0;
            schedule_build(b, err.text, sizeof err.text);
            return 1;
        }
        /* otherwise this is init's case: there is no instance left to keep */
        say(b, NULL, NULL, BRIDGE_ERR, "module '%s' is gone: its backend cannot hold two "
            "instances, so the old one had already been shut down", old->identity);
        if (cfg->on_error == BRIDGE_FAIL) stop_the_bridge(b, old->identity, "could not be reloaded");
        take_out(b, old);
        schedule_build(b, err.text, sizeof err.text);
        b->layout_moved = 1;
        store_barrier(b);
        return 1;
    }

    if (side_by_side) shut_down(b, old);
    /* what the new instance no longer declares is treated as an unload treats it */
    drop_systems(b, old);
    for (i = old->declared_count; i --;) component_release(b, old->declared[i], old->policy[i]);
    old->declared_count = 0;
    old->ids = NULL;                /* the new record's now */
    /* the settings component among them: its row was Bridge-ASM700's own, and goes too */
    for (i = 0; i < n->declared_count && n->declared[i] != n->config; i ++) {}
    if (n->config && i == n->declared_count) {
        if (ecs_is_alive(b->world, n->config_entity)) ecs_delete(b->world, n->config_entity);
        n->config = NULL;
        n->config_entity = 0;
    }
    for (link = &b->modules; *link != old; link = &(*link)->next) {}
    n->next = old->next;
    *link = n;                      /* in its place: the order modules loaded in is kept */
    module_discard(b, old);
    schedule_build(b, err.text, sizeof err.text);
    b->layout_moved = 1;
    store_barrier(b);
    return b->stopped_by ? 1 : 0;
}
