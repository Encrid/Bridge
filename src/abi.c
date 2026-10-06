/* The ABI: the eight functions a module imports. docs/modules/abi.md.
 *
 * Every pointer argument is an offset into the caller's scratch arena. An entry that cannot be
 * honoured says why in the module's record and answers non-zero, which abandons the
 * invocation that made it - docs/bridge/memory/budgets.md, *What running out does*. */
#include "inside.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int refuse(module *m, const char *fmt, ...)
{
    va_list args;
    /* the first refusal of a call is the reason: a backend that cannot unwind lets the
     * module run on, and what it meets afterwards is consequence */
    if (m->refused) return 1;
    va_start(args, fmt);
    vsnprintf(m->why, sizeof m->why, fmt, args);
    va_end(args);
    m->refused = 1;
    return 1;
}

/* the one memory a module can write that Bridge-ASM700 can read and bounds-check */
static const void *scratch(module *m, uint64_t at, uint64_t len)
{
    if (at < m->scratch || len > m->scratch_len || at - m->scratch > m->scratch_len - len) {
        return nullptr;
    }
    return m->priv + at;
}

static uint64_t fingerprint(const char *s, size_t n)
{
    uint64_t h = 1'469'598'103'934'665'603ull;       /* FNV-1a */
    size_t i;
    for (i = 0; i < n; i ++) h = (h ^ (uint8_t)s[i]) * 1'099'511'628'211ull;
    return h;
}

static int listed(char **list, uint32_t count, const char *name)
{
    uint32_t i;
    for (i = 0; i < count; i ++) {
        if (!strcmp(list[i], name)) return 1;
    }
    return 0;
}

/* grants.md: a module's own namespace is its own, and another's needs a grant */
static int foreign(module *m, const char *name)
{
    size_t n = strlen(m->ns);
    return strncmp(name, m->ns, n) || name[n] != ':';
}

static int may_write(module *m, const declared_type *c)
{
    return !foreign(m, c->name) || listed(m->write, m->write_count, c->name);
}

/* a type a field names bare is looked for in the namespace of the component it is a field of */
typedef struct { bridge *b; const char *ns; } looking;

static bool find(void *ctx, const char *bare, schema_named *named)
{
    const looking *l = ctx;
    size_t ns = strlen(l->ns);
    for (uint32_t i = 0; i < l->b->type_count; i ++) {
        declared_type *c = l->b->types[i];
        if (strncmp(c->name, l->ns, ns) || c->name[ns] != ':' || strcmp(c->name + ns + 1, bare)) continue;
        *named = (schema_named){ c->kind, c->size, c->align, c };
        return true;
    }
    return false;
}

/* declare(name, name_len, kind, size, align, desc, desc_len, on_unload) -> slot */
static int abi_declare(void *ud, const uint64_t *a, uint64_t *r)
{
    module *m = ud;
    bridge *b = m->b;
    const char *name_at = scratch(m, a[0], a[1]), *desc_at = scratch(m, a[5], a[6]);
    enum bridge_kind kind = (enum bridge_kind)a[2];
    enum bridge_on_unload on_unload = (enum bridge_on_unload)a[7];
    uint32_t size = (uint32_t)a[3], align = (uint32_t)a[4], i;
    declared_type *c = nullptr;
    char name[128], *bare;
    uint64_t print;

    if (!m->in_init) return refuse(m, "declare is called outside init");
    if (!name_at || !desc_at || !a[1] || a[1] >= sizeof name) {
        return refuse(m, "declare: a name or a descriptor lies outside the scratch arena");
    }
    memcpy(name, name_at, a[1]);
    name[a[1]] = 0;
    bare = strchr(name, ':');
    if (!bare) return refuse(m, "type '%s' has no namespace", name);
    if (kind < BRIDGE_STRUCT || kind > BRIDGE_BITMASK) {
        return refuse(m, "type '%s': its kind is unset", name);
    }
    if (foreign(m, name) && !listed(m->read, m->read_count, name)
        && !listed(m->write, m->write_count, name)) {
        return refuse(m, "it may not declare '%s': that is another namespace's, and no grant "
            "names it", name);
    }
    if (on_unload < BRIDGE_STRIP || on_unload > BRIDGE_DELETE) {
        return refuse(m, "type '%s': on_unload is unset", name);
    }
    print = fingerprint(desc_at, a[6]) ^ kind;

    for (i = 0; i < b->type_count; i ++) {
        if (!strcmp(b->types[i]->name, name)) c = b->types[i];
    }
    if (c && c->fingerprint != print) {
        return refuse(m, "type '%s' has another component layout here: fingerprint %016llx, and it "
            "was declared with %016llx", name, (unsigned long long)print,
            (unsigned long long)c->fingerprint);
    }
    if (!c) {
        /* The descriptor is read and laid out. A field's type is a bare C name, looked for in
         * the declarer's namespace: `Vec3` in game:Body is game:Vec3 - schema.md. */
        char *desc = copy(desc_at, a[6]);
        char why[200];
        looking l = { b, name };
        schema s;
        int refused;
        uint32_t k;

        *bare = 0;
        refused = schema_parse(kind, desc, find, &l, &s, why, sizeof why);
        *bare = ':';
        free(desc);
        if (refused) return refuse(m, "type '%s': its descriptor %s", name, why);
        if (s.size != size || s.align != align) {
            uint32_t laid = s.size, aligned = s.align;
            schema_free(&s);
            return refuse(m, "type '%s' is %u bytes aligned to %u in the module and %u "
                "aligned to %u by its descriptor", name, size, align, laid, aligned);
        }
        if (b->type_count >= b->max_types) {
            schema_free(&s);
            return refuse(m, "type '%s' is one more than [budget] types of %u", name,
                b->max_types);
        }

        c = calloc(1, sizeof *c);
        c->name = copy(name, strlen(name));
        c->type = storage_declare(b->storage, size);
        c->s = s;
        c->kind = kind;
        c->size = size;
        c->align = align;
        c->fingerprint = print;
        c->zero = calloc(1, size);
        b->types = realloc(b->types, (b->type_count + 1) * sizeof *b->types);
        b->types[b->type_count ++] = c;
        /* a declared type a field of this one names is held for as long as this one is */
        for (k = 0; k < s.field_count; k ++) {
            declared_type *held = s.fields[k].type;
            if (!held) continue;
            c->deps = realloc(c->deps, (c->dep_count + 1) * sizeof *c->deps);
            c->deps[c->dep_count ++] = held;
            held->users ++;
        }
    }

    for (i = 0; i < m->declared_count; i ++) {
        if (m->declared[i] == c) {              /* declare is idempotent */
            r[0] = i;
            return 0;
        }
    }
    m->declared = realloc(m->declared, (m->declared_count + 1) * sizeof *m->declared);
    m->policy = realloc(m->policy, (m->declared_count + 1) * sizeof *m->policy);
    m->policy[m->declared_count] = on_unload;
    m->declared[m->declared_count] = c;
    c->declarers ++;
    b->layout_moved = 1;
    r[0] = m->declared_count ++;
    return 0;
}

/* storage says who and what; the observer itself runs at the barrier, after the republish */
static void heard(void *ctx, uint64_t id, enum bridge_event event)
{
    bridge_system *s = ctx;
    if (s->pending_count && s->pending[s->pending_count - 1].id == id) {
        s->pending[s->pending_count - 1].events |= event;
        return;
    }
    if (s->pending_count == s->pending_room) {
        s->pending_room = s->pending_room ? s->pending_room * 2 : 16;
        s->pending = realloc(s->pending, s->pending_room * sizeof *s->pending);
    }
    s->pending[s->pending_count].id = id;
    s->pending[s->pending_count ++].events = event;
}

/* A system or an observer is a query plus a function, so one registration serves both.
 * `events` is zero for a system, and then `phase` is the phase it named. */
static int subscribe(module *m, uint64_t name_off, uint64_t name_len, uint32_t phase,
                     enum bridge_event events, uint64_t terms_off, uint64_t term_count, uint64_t *r)
{
    bridge *b = m->b;
    const char *what = events ? "observer" : "system";
    const char *name_at = scratch(m, name_off, name_len);
    const bridge_term *terms = scratch(m, terms_off, term_count * sizeof(bridge_term));
    uint32_t count = (uint32_t)term_count, i;
    storage_term *asked;
    bridge_system *s;
    char name[128], full[256];

    if (b->system_count >= b->max_systems) {
        return refuse(m, "it registers one more than [budget] systems of %u", b->max_systems);
    }
    if (term_count > b->max_terms) {
        return refuse(m, "a%s %s asks for %llu terms, which is more than [budget] terms of %u",
            events ? "n" : "", what, (unsigned long long)term_count, b->max_terms);
    }
    if (!name_at || !terms || !name_len || name_len >= sizeof name) {
        return refuse(m, "%s: a name or a term lies outside the scratch arena", what);
    }
    memcpy(name, name_at, name_len);
    name[name_len] = 0;
    if (!strcmp(name, "init") || !strcmp(name, "shutdown") || !strcmp(name, "reload")) {
        return refuse(m, "'%s' is a reserved name, and no %s may take it", name, what);
    }
    /* a system and an observer are both exports, and an export has one name */
    for (i = 0; i < m->system_count + m->observer_count; i ++) {
        bridge_system *other = i < m->system_count ? m->systems[i]
                                                   : m->observers[i - m->system_count];
        if (!strcmp(other->name, name)) return refuse(m, "'%s' is registered twice", name);
    }
    if (events && !count) {
        return refuse(m, "observer '%s' has no term, and so nothing to observe", name);
    }

    s = calloc(1, sizeof *s + count * sizeof *s->term);
    asked = calloc(count + 1, sizeof *asked);
    /* query.md, *What a query may not ask for* */
    for (i = 0; i < count; i ++) {
        bridge_term t;
        declared_type *c;
        memcpy(&t, &terms[i], sizeof t);
        if (t.slot >= m->declared_count) {
            refuse(m, "term %u of %s '%s' names slot %u, which it never declared", i, what, name,
                t.slot);
        } else if (!t.access) {
            refuse(m, "term %u of %s '%s': its access is unset", i, what, name);
        } else if (t.access > BRIDGE_WRITE) {
            refuse(m, "term %u of %s '%s': its access is %u, and there is read and write", i, what,
                name, t.access);
        } else if (!t.oper) {
            refuse(m, "term %u of %s '%s': its operator is unset", i, what, name);
        } else if (t.oper > BRIDGE_OPTIONAL) {
            refuse(m, "term %u of %s '%s': its operator is %u, and there are three", i, what, name,
                t.oper);
        } else if (t.access == BRIDGE_WRITE && !may_write(m, m->declared[t.slot])) {
            refuse(m, "%s '%s' may not write '%s': no grant says so", what, name,
                m->declared[t.slot]->name);
        }
        if (m->refused) {
            free(asked);
            free(s);
            return 1;
        }
        c = m->declared[t.slot];
        asked[i].type = c->type;
        asked[i].oper = t.oper;
        /* a term that must not match touches nothing */
        s->term[i].touches = t.oper == BRIDGE_NOT ? nullptr : c;
        s->term[i].writes = t.access == BRIDGE_WRITE;
    }

    s->matched = events ? storage_observer(b->storage, asked, count, events, heard, s)
               : count ? storage_system(b->storage, asked, count) : nullptr;
    free(asked);

    snprintf(full, sizeof full, "%s:%s", m->stem, name);
    s->m = m;
    s->name = copy(name, strlen(name));
    s->full = copy(full, strlen(full));
    s->phase = phase;
    s->terms = count;
    s->observer = events != 0;
    b->systems = realloc(b->systems, (b->system_count + 1) * sizeof *b->systems);
    b->systems[b->system_count ++] = s;
    if (events) {
        m->observers = realloc(m->observers, (m->observer_count + 1) * sizeof *m->observers);
        m->observers[m->observer_count] = s;
        r[0] = m->observer_count ++;
    } else {
        m->systems = realloc(m->systems, (m->system_count + 1) * sizeof *m->systems);
        m->systems[m->system_count] = s;
        r[0] = m->system_count ++;
    }
    return 0;
}

/* system(name, name_len, phase, phase_len, terms, term_count) -> slot */
static int abi_system(void *ud, const uint64_t *a, uint64_t *r)
{
    module *m = ud;
    bridge *b = m->b;
    const char *phase_at = scratch(m, a[2], a[3]);
    uint32_t phase;
    if (!m->in_init) return refuse(m, "system is called outside init");
    if (!phase_at) return refuse(m, "system: a phase lies outside the scratch arena");
    for (phase = 0; phase < b->phase_count; phase ++) {
        if (strlen(b->phases[phase]) == a[3] && !memcmp(b->phases[phase], phase_at, a[3])) break;
    }
    if (phase == b->phase_count) {
        return refuse(m, "a system names phase '%.*s', which the deployment did not declare",
            (int)a[3], phase_at);
    }
    return subscribe(m, a[0], a[1], phase, 0, a[4], a[5], r);
}

/* observe(name, name_len, events, terms, term_count) -> slot. It takes no phase: it has one
 * already, the barrier that applied its trigger. */
static int abi_observe(void *ud, const uint64_t *a, uint64_t *r)
{
    module *m = ud;
    if (!m->in_init) return refuse(m, "observe is called outside init");
    if (!a[2] || a[2] & ~(uint64_t)(BRIDGE_ON_ADD | BRIDGE_ON_SET | BRIDGE_ON_REMOVE)) {
        return refuse(m, "observe: its events are unset, or name one there is not");
    }
    return subscribe(m, a[0], a[1], m->b->phase_count, (uint32_t)a[2], a[3], a[4], r);
}

/* Nothing below touches storage. Modules run at once, and what one asks for must not depend on
 * who ran beside it - schedule.md, *What a pass depends on*. So an id comes from a block set
 * aside for the module, arena space from its own share, and a change goes on its own queue,
 * which the barrier applies in an order that never varies. No lock: a module's systems are
 * serialised, so each of the three has one writer. */

static void queue(module *m, enum change_op op, uint64_t id, declared_type *c, const void *value)
{
    change ch = { id, c, value ? c->size : 0, op };
    uint32_t need = (uint32_t)sizeof ch + ((ch.size + 7u) & ~7u);
    if (m->queue_len + need > m->queue_room) {
        m->queue_room = (m->queue_room + need) * 2;
        m->queue = realloc(m->queue, m->queue_room);
    }
    memcpy(m->queue + m->queue_len, &ch, sizeof ch);
    /* the value is copied now: the scratch arena it is in is reset before it is applied */
    if (value) memcpy(m->queue + m->queue_len + sizeof ch, value, ch.size);
    m->queue_len += need;
}

/* spawn() -> id */
static int abi_spawn(void *ud, [[maybe_unused]] const uint64_t *a, uint64_t *r)
{
    module *m = ud;
    if (m->id_next == m->id_count) {
        if (m->spawns <= 0) return refuse(m, "spawn: it stated that it spawns nothing");
        if (m->ids_short) {
            /* the index is twelve bytes times the highest id, and that is what was budgeted */
            return refuse(m, "spawn: [budget] entities of %u is reached", m->b->entities);
        }
        return refuse(m, "spawn: its spawns of %lld between two barriers are used up",
            (long long)m->spawns);
    }
    r[0] = m->ids[m->id_next ++];
    return 0;
}

/* destroy(id). An id that has already died is nobody's mistake: two modules may both want it
 * gone, and which of them is told at the barrier, where it is looked at. */
static int abi_destroy(void *ud, const uint64_t *a, [[maybe_unused]] uint64_t *r)
{
    queue(ud, CHANGE_DESTROY, a[0], nullptr, nullptr);
    return 0;
}

static declared_type *written(module *m, uint64_t slot, const char *entry)
{
    if (slot >= m->declared_count) {
        refuse(m, "%s names slot %llu, which it never declared", entry, (unsigned long long)slot);
        return nullptr;
    }
    if (!may_write(m, m->declared[slot])) {
        refuse(m, "it may not %s '%s': no grant says so", entry, m->declared[slot]->name);
        return nullptr;
    }
    return m->declared[slot];
}

/* attach(id, slot, ptr, len). The value travels with it; zero length means zeroed. */
static int abi_attach(void *ud, const uint64_t *a, [[maybe_unused]] uint64_t *r)
{
    module *m = ud;
    declared_type *c = written(m, a[1], "attach");
    const void *value;
    if (!c) return 1;
    value = a[3] ? scratch(m, a[2], a[3]) : c->zero;
    if (a[3] && a[3] != c->size) {
        return refuse(m, "attach hands '%s' %llu bytes and it is %u", c->name,
            (unsigned long long)a[3], c->size);
    }
    if (!value) return refuse(m, "attach: the value lies outside the scratch arena");
    queue(m, CHANGE_ATTACH, a[0], c, value);
    return 0;
}

/* detach(id, slot) */
static int abi_detach(void *ud, const uint64_t *a, [[maybe_unused]] uint64_t *r)
{
    module *m = ud;
    declared_type *c = written(m, a[1], "detach");
    if (!c) return 1;
    queue(m, CHANGE_DETACH, a[0], c, nullptr);
    return 0;
}

/* alloc(len) -> offset into the persistent arena. Nothing frees it while its module is
 * loaded - strings.md. */
static int abi_alloc(void *ud, const uint64_t *a, uint64_t *r)
{
    module *m = ud;
    uint64_t len = (a[0] + 7u) & ~7ull;
    if (m->persistent <= 0) {
        return refuse(m, "alloc: it stated that it keeps nothing in the persistent arena");
    }
    /* what is left of a range too small for this is dead space, as a string that outgrew
     * its place leaves - strings.md */
    while (len > m->arena_left && m->own_next < m->own_count) {
        m->arena_at = m->owned[m->own_next].at;
        m->arena_left = m->owned[m->own_next ++].len;
    }
    if (len > m->arena_left) {
        return refuse(m, "alloc: its persistent of %lld bytes is exhausted", (long long)m->persistent);
    }
    r[0] = m->arena_at;
    m->arena_at += (uint32_t)len;
    m->arena_left -= (uint32_t)len;
    return 0;
}

int abi_define(module *m, backend_err *err)
{
    constexpr enum backend_type I = BACKEND_I32, L = BACKEND_I64;
    static const struct {
        const char *name;
        backend_fn  fn;
        enum backend_type params[8];
        uint32_t    param_count;
        enum backend_type results[1];
        uint32_t    result_count;
    } entries[] = {
        { "declare", abi_declare, { I, I, I, I, I, I, I, I }, 8, { I }, 1 },
        { "system",  abi_system,  { I, I, I, I, I, I },    6, { I }, 1 },
        { "observe", abi_observe, { I, I, I, I, I },       5, { I }, 1 },
        { "spawn",   abi_spawn,   {},                      0, { L }, 1 },
        { "destroy", abi_destroy, { L },                   1, {}, 0 },
        { "attach",  abi_attach,  { L, I, I, I },          4, {}, 0 },
        { "detach",  abi_detach,  { L, I },                2, {}, 0 },
        { "alloc",   abi_alloc,   { I },                   1, { I }, 1 },
    };
    size_t i;
    for (i = 0; i < sizeof entries / sizeof *entries; i ++) {
        backend_sig sig = { entries[i].params, entries[i].param_count,
                            entries[i].results, entries[i].result_count };
        if (m->on->api.define_func(m->instance, BRIDGE_NAMESPACE, entries[i].name, entries[i].fn,
                                   m, &sig, err)) return 1;
    }
    return 0;
}
