/* The runtime lifecycle: load, unload, reload. docs/seam/host_api/lifecycle.md is the order of
 * each; docs/bridge/data/lifetime.md is what becomes of storage; docs/bridge/run/failure.md is
 * what a failure costs. All three are legal only between phases. */
#include "inside.h"

#include <stdckdint.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static char **copy_list(const char *const *list, uint32_t count)
{
    char **out = calloc(count ? count : 1, sizeof *out);
    uint32_t i;
    for (i = 0; out && i < count; i ++) {
        out[i] = copy(list[i], strlen(list[i]));
        if (out[i]) continue;
        while (i --) free(out[i]);
        free(out);
        return nullptr;
    }
    return out;
}

static void free_list_of(char **list, uint32_t count)
{
    uint32_t i;
    for (i = 0; list && i < count; i ++) free(list[i]);
    free(list);
}

/* ---- components leaving ---- */

/* A component's record goes: what storage knows of it, and its hold on the types its fields
 * name, which go with it when nothing else declares or names them. */
static void type_delete(bridge_t *b, declared_type_t *c)
{
    uint32_t k;
    storage_undeclare(b->storage, c->type);
    for (k = 0; b->types[k] != c; k ++) {}
    b->types[k] = b->types[-- b->type_count];
    for (k = 0; k < c->dep_count; k ++) {
        declared_type_t *named = c->deps[k];
        if (!-- named->users && !named->declarers) type_delete(b, named);
    }
    schema_free(&c->s);
    free(c->runs);
    free(c->deps);
    free(c->name);
    free(c->zero);
    free(c);
}

/* A module stops declaring `c`. Shared, it stays; the last to leave applies its own policy.
 * Policy zero is a load that failed: nothing can carry what was never applied. */
static void type_release(bridge_t *b, declared_type_t *c, enum bridge_on_unload policy)
{
    if (-- c->declarers) return;
    if (policy) storage_drop(b->storage, c->type, policy);
    /* a type some other component's field still names is kept as a type, with no rows */
    if (!c->users) type_delete(b, c);
    b->layout_moved = 1;
}

/* ---- a module's record ---- */

static void drop_systems(bridge_t *b, module_t *m)
{
    uint32_t i, kept = 0;
    for (i = 0; i < b->system_count; i ++) {
        bridge_system_t *s = b->systems[i];
        if (s->m != m) {
            b->systems[kept ++] = s;
            continue;
        }
        if (s->matched) storage_forget(b->storage, s->matched);
        if (s->parked) b->parked --;
        free(s->asked);
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
void module_discard(bridge_t *b, module_t *m)
{
    drop_systems(b, m);
    if (m->instance) m->on->api.instance_destroy(m->instance);
    if (m->image) m->on->api.module_destroy(m->image);
    if (m->memory) m->on->api.memory_release(m->memory);
    if (m->priv) os_release(m->priv, m->priv_reserve);
    free_list_of(m->read, m->read_count);
    free_list_of(m->phase_from, m->phase_names);
    free(m->phase_to);
    free_list_of(m->write, m->write_count);
    free(m->ids);
    free(m->owned);
    free(m->queue);
    free(m->marks);
    free(m->writable);
    free(m->loss_rows);
    free(m->systems);
    free(m->observers);
    free(m->declared);
    free(m->policy);
    free(m->may_write);
    free(m->identity);
    free(m->stem);
    free(m->ns);
    free(m);
}

/* A load that failed leaves storage as it was: the ids set aside for it, the row its settings
 * were on, what it took of the arena, and every component nobody else declared. What it
 * queued goes with its queue. */
static void undo(bridge_t *b, module_t *m)
{
    uint32_t i;
    drop_systems(b, m);         /* first: a query must not outlive a component it names */
    storage_release(b->storage, m->ids, m->id_count);
    m->id_count = m->id_next = 0;
    /* nothing of it was applied, so nothing points into its share */
    for (i = 0; i < m->own_count; i ++) {
        region_give(&b->arena_space, m->owned[i].at - b->arena, m->owned[i].len);
    }
    b->arena_ranges -= m->own_count;
    m->own_count = 0;
    /* last declared first: a type is used by what was declared after it */
    for (i = m->declared_count; i --;) type_release(b, m->declared[i], 0);
    m->declared_count = 0;
    b->layout_moved = 1;
}

/* ---- what can be decided from a config alone ---- */

/* A key whose zero means nobody stated it - config.md, *No unaccountable values*. */
static int unset(bridge_t *b, const char *identity, const char *key, int64_t value)
{
    if (value) return 0;
    say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is refused: %s is unset", identity, key);
    return 1;
}

/* A key that has a "none" has one, and a negative number that is not it is nobody's:
 * config.md, *No unaccountable values*. `none` is BRIDGE_NO_LIMIT or BRIDGE_NONE. */
static int neither(bridge_t *b, const char *identity, const char *key, int64_t value, int64_t none)
{
    if (value > 0 || value == none) return 0;
    say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is refused: %s is %lld, which is neither %s",
        identity, key, (long long)value, none == BRIDGE_NO_LIMIT
            ? "a limit nor BRIDGE_NO_LIMIT" : "an amount nor BRIDGE_NONE");
    return 1;
}

/* A key that is yes or no is stated as either: config.md, *No unaccountable values*. */
static int unflagged(bridge_t *b, const char *identity, const char *key, enum bridge_flag value)
{
    if (value == BRIDGE_TRUE || value == BRIDGE_FALSE) return 0;
    if (value == BRIDGE_UNSET) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is refused: %s is unset", identity, key);
    } else {
        say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is refused: %s is %u, which is neither "
            "BRIDGE_TRUE nor BRIDGE_FALSE", identity, key, value);
    }
    return 1;
}

/* A stated limit demands that the backend can enforce it - capabilities.md. */
static int lacks(bridge_t *b, const char *identity, open_backend_t *o, uint32_t flag, const char *key)
{
    if (o->capabilities & flag) return 0;
    /* a guarantee has one name, for the key that asks it and the capability that answers */
    say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is refused: it states %s, which backend '%s' "
        "cannot keep", identity, key, o->name);
    return 1;
}

/* Everything a load and a reload both refuse before the artifact is touched. */
/* Where a module's regions sit in its private memory: the header, then scratch, the log ring
 * and the spans, each at a multiple of 8. 1 if what it stated cannot be laid out in 32 bits,
 * which is what every offset into that memory is. */
static int private_layout(const bridge_module_config_t *cfg, uint64_t granule, uint32_t *ring,
                          uint32_t *spans, uint32_t *end)
{
    constexpr uint32_t scratch = 128;
    uint32_t at;
    if (ckd_add(&at, scratch, cfg->scratch) || ckd_add(&at, at, 7u)) return 1;
    *ring = at & ~7u;
    if (ckd_add(&at, *ring, cfg->log_ring) || ckd_add(&at, at, 7u)) return 1;
    *spans = at & ~7u;
    /* and rounded up to its backend's granule it is still an offset of 32 bits */
    return ckd_add(end, *spans, cfg->spans) || (uint64_t)*end + granule - 1 > UINT32_MAX;
}

static int checks(bridge_t *b, const bridge_artifact_t *a, open_backend_t **on,
                  enum backend_capability *demands)
{
    const bridge_module_config_t *cfg = &a->cfg;
    const char *id = cfg->identity;
    open_backend_t *o = nullptr;
    uint32_t i;

    if (!id || !*id) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "a module with no identity is refused");
        return 1;
    }
    for (i = 0; i < b->backend_count && a->backend; i ++) {
        if (!strcmp(b->backends[i].name, a->backend)) o = &b->backends[i];
    }
    if (!o) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is refused: no backend named '%s'", id,
            a->backend ? a->backend : "");
        return 1;
    }
    if (cfg->abi != BRIDGE_ABI) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is refused: it wants ABI %u and this is "
            "ABI %u", id, cfg->abi, (unsigned)BRIDGE_ABI);
        return 1;
    }
    if (unflagged(b, id, "trusted", cfg->trusted) || unflagged(b, id, "reloadable", cfg->reloadable)
        || unflagged(b, id, "hot_reload", cfg->hot_reload)
        || unflagged(b, id, "pinned", cfg->pinned)) {
        return 1;
    }
    /* schedule.md, *A module's phase names are mapped by the deployment* */
    for (i = 0; i < cfg->phase_count; i ++) {
        const bridge_phase_name_t *n = &cfg->phases[i];
        uint32_t k, phase;
        if (!n->from || !*n->from || !n->to || !*n->to) {
            say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is refused: phase name %u of its "
                "config is unset", id, i);
            return 1;
        }
        for (k = 0; k < i && strcmp(cfg->phases[k].from, n->from); k ++) {}
        if (k < i) {
            say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is refused: its config maps phase "
                "'%s' twice", id, n->from);
            return 1;
        }
        for (phase = 0; phase < b->phase_count && strcmp(b->phases[phase], n->to); phase ++) {}
        if (phase == b->phase_count) {
            say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is refused: its config maps phase "
                "'%s' to '%s', which the deployment did not declare", id, n->from, n->to);
            return 1;
        }
    }
    if (cfg->on_error != BRIDGE_FAIL && cfg->on_error != BRIDGE_SKIP) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is refused: on_error is unset", id);
        return 1;
    }
    if (unset(b, id, "max_system_failures", cfg->max_system_failures)
        || unset(b, id, "work_limit", cfg->work_limit) || unset(b, id, "memory_limit", cfg->memory_limit)
        || unset(b, id, "init_timeout", cfg->init_timeout)
        || unset(b, id, "reload_timeout", cfg->reload_timeout)
        || unset(b, id, "shutdown_timeout", cfg->shutdown_timeout)
        || unset(b, id, "scratch", cfg->scratch) || unset(b, id, "log_ring", cfg->log_ring)
        || unset(b, id, "spans", cfg->spans) || unset(b, id, "spawns", cfg->spawns)
        || unset(b, id, "persistent", cfg->persistent)
        || unset(b, id, "changes", cfg->changes) || unset(b, id, "loss_count", cfg->loss_count)) {
        return 1;
    }
    if (cfg->loss_count > 0 && !cfg->losses) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is refused: it states %lld losses, and "
            "losses is unset", id, (long long)cfg->loss_count);
        return 1;
    }
    if (neither(b, id, "max_system_failures", cfg->max_system_failures, BRIDGE_NO_LIMIT)
        || neither(b, id, "work_limit", cfg->work_limit, BRIDGE_NO_LIMIT)
        || neither(b, id, "memory_limit", cfg->memory_limit, BRIDGE_NO_LIMIT)
        || neither(b, id, "init_timeout", cfg->init_timeout, BRIDGE_NO_LIMIT)
        || neither(b, id, "reload_timeout", cfg->reload_timeout, BRIDGE_NO_LIMIT)
        || neither(b, id, "shutdown_timeout", cfg->shutdown_timeout, BRIDGE_NO_LIMIT)
        || neither(b, id, "spawns", cfg->spawns, BRIDGE_NONE)
        || neither(b, id, "persistent", cfg->persistent, BRIDGE_NONE)
        || neither(b, id, "changes", cfg->changes, BRIDGE_NONE)
        || neither(b, id, "loss_count", cfg->loss_count, BRIDGE_NONE)) {
        return 1;
    }
    if (cfg->changes > (int64_t)UINT32_MAX) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is refused: its changes of %lld bytes is "
            "more than an offset of 32 bits can reach", id, (long long)cfg->changes);
        return 1;
    }
    /* log.md: zero is unset for it as for every key, so the least it can be is 1 */
    if (unset(b, id, "log_level", cfg->log_level)) return 1;
    if (cfg->log_level > BRIDGE_LEVEL_MOST) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is refused: its log_level of %u is not "
            "one it can be, which is 1 to %u", id, cfg->log_level, BRIDGE_LEVEL_MOST);
        return 1;
    }
    if (!cfg->config || !*cfg->config) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is refused: config is unset", id);
        return 1;
    }
    if (cfg->spawns > (int64_t)b->entities) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is refused: its spawns of %lld is more "
            "than [budget] entities of %u", id, (long long)cfg->spawns, b->entities);
        return 1;
    }
    uint32_t ring, spans, end;
    if (private_layout(cfg, o->granule, &ring, &spans, &end)) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is refused: its scratch of %u, log_ring of "
            "%u and spans of %u bytes come to more than an offset of 32 bits can reach", id,
            cfg->scratch, cfg->log_ring, cfg->spans);
        return 1;
    }
    /* budgets.md: the one floor with an argument. A module that can report nothing is the one
     * legal number that makes a deployment incoherent. */
    if (cfg->log_ring < sizeof(bridge_log_record_t)) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is refused: a log_ring of %u bytes holds "
            "no record, and one is %u", id, cfg->log_ring, (unsigned)sizeof(bridge_log_record_t));
        return 1;
    }
    /* capabilities.md: one guarantee, asked by `trusted` and answered by the backend */
    if (cfg->trusted == BRIDGE_FALSE && !(o->capabilities & BACKEND_UNTRUSTED)) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is refused: it is not trusted, and backend "
            "'%s' runs only modules that are", id, o->name);
        return 1;
    }
    if ((cfg->work_limit > 0 && lacks(b, id, o, BACKEND_WORK_LIMIT, "work_limit"))
        || (cfg->memory_limit > 0 && lacks(b, id, o, BACKEND_MEMORY_LIMIT, "memory_limit"))
        || (cfg->init_timeout > 0 && lacks(b, id, o, BACKEND_TIMEOUT, "init_timeout"))
        || (cfg->reload_timeout > 0 && lacks(b, id, o, BACKEND_TIMEOUT, "reload_timeout"))
        || (cfg->shutdown_timeout > 0
            && lacks(b, id, o, BACKEND_TIMEOUT, "shutdown_timeout"))
        || (cfg->hot_reload == BRIDGE_TRUE && lacks(b, id, o, BACKEND_RELOAD, "hot_reload"))) {
        return 1;
    }
    /* wasm.md, *Exports*: the author says "cannot" and the deployment "may not", each with a
     * key. A deployment allowing what the author says cannot be done is config against config. */
    if (cfg->hot_reload == BRIDGE_TRUE && cfg->reloadable == BRIDGE_FALSE) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is refused: hot_reload is stated, and its "
            "author states it is not reloadable", id);
        return 1;
    }
    *on = o;
    *demands = (cfg->trusted == BRIDGE_TRUE ? 0 : BACKEND_UNTRUSTED) | (cfg->work_limit > 0 ? BACKEND_WORK_LIMIT : 0)
             | (cfg->memory_limit > 0 ? BACKEND_MEMORY_LIMIT : 0)
             | (cfg->init_timeout > 0 || cfg->reload_timeout > 0 || cfg->shutdown_timeout > 0
                ? BACKEND_TIMEOUT : 0);
    return 0;
}

static size_t stem_of(const char *identity)
{
    const char *at = strchr(identity, '@');
    return at ? (size_t)(at - identity) : strlen(identity);
}

/* a system is named by its module without the version, so that is what tells modules apart -
 * schedule.md, *Order within a phase is never implicit* */
static module_t *loaded(bridge_t *b, const char *identity)
{
    size_t stem = stem_of(identity);
    module_t *m;
    for (m = b->modules; m; m = m->next) {
        if (strlen(m->stem) == stem && !memcmp(m->stem, identity, stem)) return m;
    }
    return nullptr;
}

static module_t *record(bridge_t *b, const bridge_artifact_t *a, open_backend_t *o)
{
    const bridge_module_config_t *cfg = &a->cfg;
    const char *id = cfg->identity, *colon = strchr(id, ':');
    module_t *m = calloc(1, sizeof *m);
    bool whole;
    if (bridge_short(b, m, "a module's record")) return nullptr;
    m->b = b;
    m->on = o;
    m->identity = copy(id, strlen(id));
    m->stem = copy(id, stem_of(id));
    m->ns = copy(id, colon ? (size_t)(colon - id) : strlen(id));
    m->pinned = cfg->pinned == BRIDGE_TRUE;
    m->thread = os_thread_id();
    m->on_error = cfg->on_error;
    m->hot_reload = cfg->hot_reload == BRIDGE_TRUE;
    m->max_failures = cfg->max_system_failures;
    m->shutdown_timeout = cfg->shutdown_timeout;
    m->work_limit = cfg->work_limit;
    m->spawns = cfg->spawns;
    m->persistent = cfg->persistent;
    m->losses = cfg->losses;
    m->loss_count = cfg->loss_count > 0 ? (uint32_t)cfg->loss_count : 0;
    m->loss_rows = calloc(m->loss_count ? m->loss_count : 1, sizeof *m->loss_rows);
    /* budgets.md, *Nothing is allocated in a phase*: its ids and its queue, as long as it
     * stated, now */
    whole = m->identity && m->stem && m->ns && m->loss_rows;
    if (cfg->spawns > 0) {
        m->ids = calloc((size_t)cfg->spawns, sizeof *m->ids);
        m->id_room = m->ids ? (uint32_t)cfg->spawns : 0;
        whole = whole && m->ids;
    }
    if (cfg->changes > 0) {
        m->queue = malloc((size_t)cfg->changes);
        m->queue_room = m->queue ? (uint32_t)cfg->changes : 0;
        whole = whole && m->queue;
    }
    m->phase_from = calloc(cfg->phase_count ? cfg->phase_count : 1, sizeof *m->phase_from);
    m->phase_to = calloc(cfg->phase_count ? cfg->phase_count : 1, sizeof *m->phase_to);
    whole = whole && m->phase_from && m->phase_to;
    for (m->phase_names = 0; whole && m->phase_names < cfg->phase_count; m->phase_names ++) {
        const bridge_phase_name_t *n = &cfg->phases[m->phase_names];
        uint32_t phase = 0;
        while (strcmp(b->phases[phase], n->to)) phase ++;       /* checks() found it */
        m->phase_from[m->phase_names] = copy(n->from, strlen(n->from));
        m->phase_to[m->phase_names] = phase;
        if (!m->phase_from[m->phase_names]) whole = false;
    }
    if (!m->phase_from) m->phase_names = 0;
    m->read = copy_list(cfg->read, cfg->read_count);
    m->read_count = m->read ? cfg->read_count : 0;
    m->write = copy_list(cfg->write, cfg->write_count);
    m->write_count = m->write ? cfg->write_count : 0;
    /* failure.md, *Out of memory*: all of a module's record, or none of it */
    if (!whole || !m->read || !m->write) {
        (void)bridge_short(b, nullptr, "a module's record");
        module_discard(b, m);
        return nullptr;
    }
    return m;
}

/* The private block: the header at 0, then the three regions its config sized. */
static int private_memory(module_t *m, const bridge_module_config_t *cfg, backend_err_t *err)
{
    constexpr uint32_t scratch = 128;
    uint32_t ring, spans, end;
    uint64_t len, reserve;
    /* checks() refused a config this could not lay out, so it cannot fail here */
    if (private_layout(cfg, m->on->granule, &ring, &spans, &end)) unreachable();
    len = ((uint64_t)end + m->on->granule - 1) / m->on->granule * m->on->granule;
    /* only its own backend maps a module's private block, so it has that backend's shape */
    reserve = (len > m->on->reservation ? len : m->on->reservation) + m->on->guard;
    m->priv = os_reserve((size_t)reserve, (size_t)len);
    if (!m->priv) {
        snprintf(err->text, sizeof err->text, "the operating system would not give its private memory "
            "of %llu bytes", (unsigned long long)len);
        return 1;
    }
    m->priv_len = (size_t)len;
    m->priv_reserve = (size_t)reserve;
    m->scratch = scratch;
    m->scratch_len = cfg->scratch;
    m->ring = ring;
    m->ring_len = cfg->log_ring / (uint32_t)sizeof(bridge_log_record_t);
    m->spans = spans;
    m->spans_len = cfg->spans;
    ((bridge_header_t *)m->priv)->log_level = cfg->log_level;
    ((bridge_header_t *)m->priv)->system = UINT32_MAX;        /* no system is running */
    m->memory = m->on->api.memory_adopt(m->on->be, m->priv, len, err);
    return !m->memory;
}

/* An export Bridge-ASM700 calls: found once. Whether it has the shape `call` runs is the
 * backend's to see and to refuse - signatures.md. */
static backend_func_t *export_of(module_t *m, const char *name, backend_err_t *err)
{
    return m->on->api.instance_func(m->instance, name, err);
}

/* One call of a lifecycle export, under its timeout. 0 if it returned 0. */
static int lifecycle_call(module_t *m, backend_func_t *f, const char *name, int64_t timeout,
                          backend_err_t *err)
{
    int32_t status = 0;
    int trapped;
    if (timeout > 0) m->on->api.instance_set_timeout(m->instance, (uint64_t)timeout);
    m->refused = 0;
    /* threads.md: what it changes is applied after the last barrier's observers and before
     * the next phase's systems, in the order the calls were made */
    m->turn = TURN_LIFECYCLE | (m->b->turns ++ & 0xffffu);
    if (!abi_marks(m, 0)) {
        snprintf(err->text, sizeof err->text, "out of memory: there was none to call its %s with", name);
        return 1;
    }
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
static int start(bridge_t *b, module_t *m, const bridge_artifact_t *a, enum backend_capability demands,
                 const char *entry, backend_err_t *err)
{
    const bridge_module_config_t *cfg = &a->cfg;
    open_backend_t *o = m->on;
    backend_import_t imports[64];
    backend_memory_import_t memories[2];
    uint32_t i, import_count, memory_count = 0;
    backend_func_t *first;
    uint64_t need;
    int reloading = !strcmp(entry, "reload");

    m->image = o->api.module_compile(o->be, a->bytes, a->len, a->path, demands, err);
    if (!m->image) return 1;
    if (private_memory(m, cfg, err)) return 1;
    block_header(b, m);

    /* a module is handed the memories it imports, and no other - backend.md */
    import_count = o->api.module_imports(m->image, imports, 64);
    for (i = 0; i < import_count && i < 64; i ++) {
        const backend_import_t *im = &imports[i];
        if (im->kind != BACKEND_IMPORT_MEMORY || im->ns_len != strlen(BRIDGE_NAMESPACE)
            || memcmp(im->ns, BRIDGE_NAMESPACE, im->ns_len) || memory_count == 2) continue;
        if (im->field_len == strlen(BRIDGE_SHARED) && !memcmp(im->field, BRIDGE_SHARED, im->field_len)) {
            memories[memory_count ++] = (backend_memory_import_t){ BRIDGE_NAMESPACE, BRIDGE_SHARED, o->shared };
        } else if (im->field_len == strlen(BRIDGE_PRIVATE)
                   && !memcmp(im->field, BRIDGE_PRIVATE, im->field_len)) {
            memories[memory_count ++] = (backend_memory_import_t){ BRIDGE_NAMESPACE, BRIDGE_PRIVATE, m->memory };
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

    /* lifetime.md: a loss that is accepted is one that is made. A name nothing was lost under
     * is refused, so that a list is not left where it is from one reload to the next */
    for (i = 0; i < m->loss_count; i ++) {
        if (m->loss_rows[i]) continue;
        snprintf(err->text, sizeof err->text, "it accepts the loss of '%.100s', and loses nothing "
            "of that name", m->losses[i]);
        return 1;
    }

    /* config.md: one row, written after init has declared the struct it is a row of */
    if (!reloading && config_seed(b, m, cfg, err->text, sizeof err->text)) return 1;

    /* a system is an export, found once and checked here rather than on its first frame */
    for (i = 0; i < m->system_count + m->observer_count; i ++) {
        bridge_system_t *s = i < m->system_count ? m->systems[i] : m->observers[i - m->system_count];
        s->fn = export_of(m, s->name, err);
        if (!s->fn) return 1;
    }
    /* the three lifecycle exports are every module's: none is left out to mean something */
    m->shutdown = export_of(m, "shutdown", err);
    if (!m->shutdown || !export_of(m, "reload", err)) return 1;

    /* budgets.md: column_base has a cell for each of its components in every table there may
     * be, so whether its spans hold it is known here, and not at some barrier */
    need = sizeof(bridge_query_t) * m->system_count + sizeof(bridge_list_t) * m->observer_count
         + 4ull * m->declared_count * storage_tables(b->storage) + 4ull * m->declared_count;
    if (need > m->spans_len) {
        snprintf(err->text, sizeof err->text, "its spans of %u bytes do not hold a column_base of "
            "%u components by %u tables, which with its queries is %llu bytes", m->spans_len,
            m->declared_count, storage_tables(b->storage), (unsigned long long)need);
        return 1;
    }
    return 0;
}

/* ---- load ---- */

int bridge_load(bridge_t *b, const bridge_artifact_t *a)
{
    const bridge_module_config_t *cfg = &a->cfg;
    const char *id = cfg->identity;
    open_backend_t *o = nullptr;
    backend_err_t err = {};
    enum backend_capability demands = 0;
    module_t *m, *other;

    if (bridge_stopped(b, id) || checks(b, a, &o, &demands)) return 1;
    other = loaded(b, id);
    if (other) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is refused: '%s' is loaded already", id,
            other->identity);
        return 1;
    }
    if (b->module_count >= b->max_modules) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is refused: [budget] modules of %u is "
            "reached", id, b->max_modules);
        return 1;
    }
    if (cfg->persistent > 0 && (uint64_t)cfg->persistent > region_most(&b->arena_space)) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is refused: its persistent of %lld bytes "
            "does not fit in the %u left of [budget] persistent", id, (long long)cfg->persistent,
            region_most(&b->arena_space));
        return 1;
    }

    m = record(b, a, o);
    if (!m) return 1;
    if (cfg->persistent > 0) {
        /* its share of the arena, for as long as it is loaded: nobody else allocates from it */
        uint32_t share = ((uint32_t)cfg->persistent + 7u) & ~7u, at;
        m->owned = calloc(1, sizeof *m->owned);
        if (!m->owned || !region_room(&b->arena_space, b->arena_ranges + 1)) {
            (void)bridge_short(b, nullptr, "a module's share of the persistent arena");
            module_discard(b, m);
            return 1;
        }
        if (share > region_most(&b->arena_space)) share = region_most(&b->arena_space);
        if (!region_take(&b->arena_space, share, &at)) unreachable();
        b->arena_ranges ++;
        m->owned[0] = (range_t){ b->arena + at, share };
        m->own_count = m->own_next = 1;
        m->arena_share = share;
        m->arena_at = b->arena + at;
        m->arena_left = share;
    }

    /* from here a failure is the module's, and its on_error decides what it costs. init may
     * spawn, so its ids are set aside before it runs. */
    if (block_refill(b, m)) {
        snprintf(err.text, sizeof err.text, "its spawns of %lld do not fit under [budget] "
            "entities of %u", (long long)cfg->spawns, b->entities);
        goto failed;
    }
    if (start(b, m, a, demands, "init", &err)) goto failed;
    /* lifecycle.md: refuse-and-unwind if a new system or observer conflicts unordered */
    if (schedule_build(b, err.text, sizeof err.text)) goto failed;

    /* at the end: the order modules loaded in is the order their changes are applied in */
    {
        module_t **last = &b->modules;
        while (*last) last = &(*last)->next;
        *last = m;
    }
    b->module_count ++;
    b->layout_moved = 1;
    bridge_resume(b, m);
    block_barrier(b);           /* what init queued is applied, and its spans are published */
    /* the module is in, and nothing will ever run it: that is not a load that succeeded */
    return b->stopped_by ? 1 : 0;

failed:
    say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' did not load: %s", id, err.text);
    if (cfg->on_error == BRIDGE_FAIL) bridge_stop(b, id, "failed");
    undo(b, m);
    module_discard(b, m);
    /* a schedule with this module's systems taken back out is one that was built before, and
     * so is every rebuild below whose answer is dropped: none of them can be refused, and none
     * asks for memory, which was had at create */
    (void)schedule_build(b, err.text, sizeof err.text);
    return 1;
}

/* ---- unload ---- */

/* failure.md: a shutdown that fails is logged, and the teardown continues. Nothing else is
 * coherent, and it is not tried again. */
void module_shut_down(bridge_t *b, module_t *m)
{
    backend_err_t err = {};
    /* a module whose reload failed after its old instance went has no code left to call */
    if (m->instance && lifecycle_call(m, m->shutdown, "shutdown", m->shutdown_timeout, &err)) {
        say(b, m, nullptr, BRIDGE_ERR, "module '%s': %s; its teardown continues", m->identity, err.text);
    }
    storage_apply(b->storage);        /* what it had asked for, shutdown's requests included */
    block_drain(b, m);          /* and what it said: its ring is about to go with it */
}

/* strings.md, *It is given back when nothing points into it*. A string a component keeps is a
 * bridge_text_t field, so every reference there is can be found: in a field, in an array of
 * them, or in a struct a field holds. */
typedef struct { const bridge_t *b; const module_t *m; const declared_type_t *c; uint32_t rows; } pointing_t;

static bool points_in(const pointing_t *p, const declared_type_t *c, const uint8_t *bytes)
{
    for (uint32_t k = 0; k < c->s.field_count; k ++) {
        const schema_field_t *f = &c->s.fields[k];
        const declared_type_t *held = f->type;
        if (f->kind != SCHEMA_TEXT && f->kind != SCHEMA_STRUCT) continue;
        for (uint32_t i = 0; i < f->count; i ++) {
            bridge_text_t text;
            if (f->kind == SCHEMA_STRUCT) {
                if (points_in(p, held, bytes + f->offset + (size_t)i * held->size)) return true;
                continue;
            }
            memcpy(&text, bytes + f->offset + (size_t)i * sizeof text, sizeof text);
            for (uint32_t o = 0; text.len && o < p->m->own_count; o ++) {
                const range_t *own = &p->m->owned[o];
                if (text.at < own->at + own->len && own->at < (uint64_t)text.at + text.len) return true;
            }
        }
    }
    return false;
}

/* counts, and never answers that it found one: every row is to be read */
static bool row_points_in(void *ctx, const uint8_t *row)
{
    pointing_t *p = ctx;
    p->rows += points_in(p, p->c, row);
    return false;
}

/* strings.md, *It is given back when nothing points into it*. A share that stays is said,
 * with what holds it: a later load may be refused for the room it keeps. */
static bool share_is_pointed_into(bridge_t *b, const module_t *m)
{
    pointing_t p = { b, m, nullptr, 0 };
    uint64_t bytes = 0;
    bool held = false;
    for (uint32_t i = 0; i < m->own_count; i ++) bytes += m->owned[i].len;
    for (uint32_t i = 0; i < b->type_count; i ++) {
        p.c = b->types[i];
        p.rows = 0;
        if (p.c->kind == BRIDGE_STRUCT) (void)storage_any(b->storage, p.c->type, row_points_in, &p);
        if (!p.rows) continue;
        held = true;
        say(b, nullptr, nullptr, BRIDGE_WARN, "module '%s' is unloaded and its share of the "
            "persistent arena, %llu bytes, stays: '%s' holds a string in it, in this many rows: %u",
            m->identity, (unsigned long long)bytes, p.c->name, p.rows);
    }
    return held;
}

/* The rest of an unload, once the module's code has run for the last time. */
static void take_out(bridge_t *b, module_t *m)
{
    module_t **link = &b->modules;
    uint32_t i;
    drop_systems(b, m);
    /* last declared first: a type is used by what was declared after it */
    for (i = m->declared_count; i --;) type_release(b, m->declared[i], m->policy[i]);
    m->declared_count = 0;
    /* the ids held ready for it are entities nobody will ever spawn */
    storage_release(b->storage, m->ids + m->id_next, m->id_count - m->id_next);
    /* its share of the arena: back to it, unless a row that is staying holds a string there */
    if (m->own_count && !share_is_pointed_into(b, m)) {
        for (i = 0; i < m->own_count; i ++) {
            region_give(&b->arena_space, m->owned[i].at - b->arena, m->owned[i].len);
        }
        b->arena_ranges -= m->own_count;
        config_forget(b, m);
    } else {
        config_keep(b, m);
    }
    while (*link != m) link = &(*link)->next;
    *link = m->next;
    b->module_count --;
    module_discard(b, m);
}

int bridge_unload(bridge_t *b, const char *identity)
{
    char why[256];
    module_t *m;
    /* a stopped bridge takes an unload of the module that stopped it, and of no other */
    if (bridge_stopped(b, identity)) return 1;
    for (m = b->modules; m && (!identity || strcmp(m->identity, identity)); m = m->next) {}
    if (!m) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "no module '%s' is loaded, so none is unloaded",
            identity ? identity : "");
        return 1;
    }
    bridge_given_up(b, m);
    drop_systems(b, m);         /* out of the schedule before its last call */
    module_shut_down(b, m);
    take_out(b, m);
    (void)schedule_build(b, why, sizeof why);
    b->layout_moved = 1;
    block_barrier(b);           /* what left is out of every module's spans, and observed */
    return 0;
}

/* ---- reload ---- */

/* lifetime.md, *When a component is moved*: every type this reload made in the place of
 * another takes that one's rows, all of them or none. 1 and why if there is no room. */
static int moved_rows(bridge_t *b, module_t *n, backend_err_t *err)
{
    storage_move_t moves[64];
    storage_run_t whole[64];
    uint32_t count = 0, i;
    for (i = 0; i < n->declared_count && count < 64; i ++) {
        declared_type_t *c = n->declared[i];
        if (!c->replaces) continue;
        whole[count] = (storage_run_t){ 0, 0, c->size < c->replaces->size ? c->size : c->replaces->size };
        moves[count] = (storage_move_t){ c->replaces->type, c->type, &whole[count], 1 };
        /* a component layout that changed: field by field, as declare worked it out */
        if (c->replaces->runs) {
            moves[count].runs = c->replaces->runs;
            moves[count].run_count = c->replaces->run_count;
        }
        count ++;
    }
    if (!count || storage_convert(b->storage, moves, count)) {
        for (i = 0; i < n->declared_count; i ++) n->declared[i]->replaces = nullptr;
        /* lifetime.md: what went, by the names it was accepted under */
        for (i = 0; i < n->loss_count; i ++) {
            say(b, n, nullptr, BRIDGE_WARN, "module '%s' is reloaded, and '%s' is lost from %u rows, "
                "as it accepted", n->identity, n->losses[i], n->loss_rows[i] - 1);
        }
        return 0;
    }
    snprintf(err->text, sizeof err->text, "what it keeps another way, %u of its components, "
        "could not be moved: [budget] tables of %u or [budget] columns of %u bytes has no room for "
        "the rows where they are to go, while they are still where they were", count, b->tables,
        b->columns_len);
    return 1;
}

int bridge_reload(bridge_t *b, const bridge_artifact_t *a)
{
    const bridge_module_config_t *cfg = &a->cfg;
    const char *id = cfg->identity;
    open_backend_t *o = nullptr;
    backend_err_t err = {};
    enum backend_capability demands = 0;
    uint32_t i, extra = 0, extra_at = 0;
    range_t last = {};
    bool in_place = false;
    int64_t had, wants;
    module_t *old, *n, **link;
    int side_by_side;

    if (bridge_stopped(b, id) || checks(b, a, &o, &demands)) return 1;
    old = loaded(b, id);
    if (!old) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is not reloaded: nothing of that name is "
            "loaded", id);
        return 1;
    }
    /* wasm.md, *Exports*: hot_reload only ever forbids */
    if (!old->hot_reload) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is not reloaded: its hot_reload is false",
            old->identity);
        return 1;
    }
    if (o != old->on) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is not reloaded: it runs on backend '%s' "
            "and a reload does not move it to '%s'", old->identity, old->on->name, o->name);
        return 1;
    }
    /* budgets.md: a budget changes by reload, and this one only upwards: what it has allocated
     * is in its share, and stays there for as long as it is loaded */
    had = old->persistent > 0 ? old->persistent : 0;
    wants = cfg->persistent > 0 ? cfg->persistent : 0;
    if (wants < had) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is not reloaded: its persistent is %lld "
            "and a reload states less, which a loaded module's share cannot become", old->identity, (long long)had);
        return 1;
    }
    if (wants > had) {
        extra = (((uint32_t)wants + 7u) & ~7u) - old->arena_share;
        /* where its last range stands, if that is the one it allocates from and what follows
         * it is free; and otherwise a range more */
        if (old->own_count && old->own_next == old->own_count) {
            last = old->owned[old->own_count - 1];
            in_place = region_grow(&b->arena_space, last.at - b->arena, last.len, last.len + extra);
        }
        if (!in_place && !region_room(&b->arena_space, b->arena_ranges + 1)) {
            return bridge_short(b, nullptr, "one more range of the persistent arena");
        }
        if (!in_place && !region_take(&b->arena_space, extra, &extra_at)) {
            say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' is not reloaded: its persistent of %lld "
                "bytes does not fit in the %u left of [budget] persistent", old->identity,
                (long long)wants, region_most(&b->arena_space) + old->arena_share);
            return 1;
        }
        if (!in_place) b->arena_ranges ++;
    }

    /* The new instance takes over what was set aside for the module. Until the swap these
     * are copies: a reload that fails leaves the old record's own as they were. */
    n = record(b, a, o);
    if (n) n->owned = calloc(old->own_count + 1, sizeof *n->owned);
    /* the ids held ready for the old instance are the new one's from the swap on: a copy of
     * them, in an array of its own that holds them all, whatever `spawns` it now states */
    if (n && n->id_room < old->id_count) {
        uint64_t *ids = realloc(n->ids, old->id_count * sizeof *ids);
        if (ids) {
            n->ids = ids;
            n->id_room = old->id_count;
        }
    }
    if (!n || !n->owned || n->id_room < old->id_count) {
        /* failure.md, *Out of memory*: nothing of the old instance was touched */
        if (n) {
            (void)bridge_short(b, nullptr, "a reloaded module's record");
            module_discard(b, n);
        }
        if (in_place) region_give(&b->arena_space, last.at - b->arena + last.len, extra);
        else if (extra) {
            region_give(&b->arena_space, extra_at, extra);
            b->arena_ranges --;
        }
        return 1;
    }
    if (old->id_count) memcpy(n->ids, old->ids, old->id_count * sizeof *n->ids);
    n->id_count = old->id_count;
    n->id_next = old->id_next;
    n->arena_share = old->arena_share + extra;
    n->arena_at = old->arena_at;
    n->arena_left = old->arena_left;
    if (old->own_count) memcpy(n->owned, old->owned, old->own_count * sizeof *n->owned);
    n->own_count = old->own_count;
    n->own_next = old->own_next;
    if (in_place) {
        n->owned[n->own_count - 1].len += extra;
        n->arena_left += extra;
    } else if (extra) {
        n->owned[n->own_count ++] = (range_t){ b->arena + extra_at, extra };
        if (!old->own_count) {              /* it had none: this is where it starts */
            n->arena_at = b->arena + extra_at;
            n->arena_left = extra;
            n->own_next = 1;
        }
    }
    n->config = old->config;
    n->text_at = old->text_at;
    n->text_room = old->text_room;

    /* the old instance's systems stand aside, and out of storage: the new one registers the
     * same names, in the places these had - budgets.md, *Budgets* */
    for (i = 0; i < b->system_count; i ++) {
        if (b->systems[i]->m == old) abi_park(b, b->systems[i]);
    }
    side_by_side = (o->capabilities & BACKEND_SIDE_BY_SIDE) != 0;
    if (!side_by_side) {
        /* a process cannot hold two copies of one library: the old one goes first */
        module_shut_down(b, old);
        if (old->instance) o->api.instance_destroy(old->instance);
        if (old->image) o->api.module_destroy(old->image);
        old->instance = nullptr;
        old->image = nullptr;
    }

    n->reloads = old;
    if (start(b, n, a, demands, "reload", &err) || schedule_build(b, err.text, sizeof err.text)
        || moved_rows(b, n, &err)) {
        say(b, nullptr, nullptr, BRIDGE_ERR, "module '%s' was not reloaded: %s", id, err.text);
        drop_systems(b, n);
        /* what it would have moved stays as it is, under the type it has */
        for (i = 0; i < n->declared_count; i ++) {
            if (n->declared[i]->replaces) n->declared[i]->replaces->superseded = false;
        }
        for (i = n->declared_count; i --;) type_release(b, n->declared[i], 0);
        n->declared_count = 0;
        n->id_count = n->id_next = 0;  /* the entities are the old record's still */
        /* what the reload added goes back: nothing was allocated from it that lived */
        if (in_place) region_give(&b->arena_space, last.at - b->arena + last.len, extra);
        else if (extra) {
            region_give(&b->arena_space, extra_at, extra);
            b->arena_ranges --;
        }
        module_discard(b, n);
        if (side_by_side) {
            /* failure.md: the old instance is kept, and its systems are put back as they
             * were. The new one's were dropped above, so the places are there */
            for (i = 0; i < b->system_count; i ++) abi_unpark(b, b->systems[i]);
            (void)schedule_build(b, err.text, sizeof err.text);
            return 1;
        }
        /* There is no instance left to keep. The module stays, stopped, with everything it
         * declared: a reload is a call that means "keep running", and those never discard -
         * lifetime.md. An artifact that works brings it back; an unload is what ends it. */
        drop_systems(b, old);
        old->stopped = 1;
        say(b, old, nullptr, BRIDGE_ERR, "module '%s' is stopped: its reload failed after its old "
            "instance had been shut down. What it declared is kept", old->identity);
        if (old->on_error == BRIDGE_FAIL) bridge_stop(b, old->identity, "could not be reloaded");
        (void)schedule_build(b, err.text, sizeof err.text);
        return 1;
    }

    if (side_by_side) module_shut_down(b, old);
    /* what the new instance no longer declares is treated as an unload treats it */
    drop_systems(b, old);
    for (i = old->declared_count; i --;) type_release(b, old->declared[i], old->policy[i]);
    old->declared_count = 0;
    old->id_count = old->id_next = 0;  /* the entities are the new record's now */
    /* the settings component among them: its value went by its policy, just above */
    for (i = 0; i < n->declared_count && n->declared[i] != n->config; i ++) {}
    if (n->config && i == n->declared_count) n->config = nullptr;
    for (link = &b->modules; *link != old; link = &(*link)->next) {}
    n->next = old->next;
    *link = n;                      /* in its place: the order modules loaded in is kept */
    module_discard(b, old);
    (void)schedule_build(b, err.text, sizeof err.text);
    b->layout_moved = 1;
    bridge_resume(b, n);
    block_barrier(b);
    return b->stopped_by ? 1 : 0;
}
