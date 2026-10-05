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
        return NULL;
    }
    return m->priv + at;
}

static uint64_t fingerprint(const char *s, size_t n)
{
    uint64_t h = 1469598103934665603ull;       /* FNV-1a */
    size_t i;
    for (i = 0; i < n; i ++) h = (h ^ (uint8_t)s[i]) * 1099511628211ull;
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

static int may_write(module *m, const component *c)
{
    return !foreign(m, c->name) || listed(m->write, m->write_count, c->name);
}

/* schema.md, *The type set*: flecs', minus anything pointer-sized or pointer-holding */
static const char *outside_type_set(ecs_world_t *world, ecs_entity_t type)
{
    const EcsType *t = ecs_get(world, type, EcsType);
    if (!t) return "a type nothing describes";
    switch (t->kind) {
    case EcsPrimitiveType: {
        ecs_primitive_kind_t kind = ecs_get(world, type, EcsPrimitive)->kind;
        return kind == EcsString || kind == EcsUPtr || kind == EcsIPtr
            ? "a pointer-sized type" : NULL;
    }
    case EcsStructType: {
        const EcsStruct *s = ecs_get(world, type, EcsStruct);
        ecs_member_t *members = ecs_vec_first(&s->members);
        int32_t i, count = ecs_vec_count(&s->members);
        for (i = 0; i < count; i ++) {
            const char *why = outside_type_set(world, members[i].type);
            if (why) return why;
        }
        return NULL;
    }
    case EcsArrayType:
        return outside_type_set(world, ecs_get(world, type, EcsArray)->type);
    case EcsEnumType:
    case EcsBitmaskType:
        return NULL;
    default:
        return "a vector or an opaque type";
    }
}

/* declare(name, name_len, kind, size, align, desc, desc_len, on_unload) -> slot */
static int abi_declare(void *ud, const uint64_t *a, uint64_t *r)
{
    static const ecs_type_kind_t kinds[] = { 0, EcsStructType, EcsEnumType, EcsBitmaskType };
    static const char *const kind_names[] = { "", "a struct", "an enum", "a bitmask" };
    module *m = ud;
    bridge *b = m->b;
    const char *name_at = scratch(m, a[0], a[1]), *desc_at = scratch(m, a[5], a[6]);
    uint32_t kind = (uint32_t)a[2], size = (uint32_t)a[3], align = (uint32_t)a[4];
    uint32_t on_unload = (uint32_t)a[7], i;
    component *c = NULL;
    char name[128], *bare;
    uint64_t print;

    if (!m->in_init) return refuse(m, "declare is called outside init");
    if (!name_at || !desc_at || !a[1] || a[1] >= sizeof name) {
        return refuse(m, "declare: a name or a descriptor lies outside the scratch arena");
    }
    memcpy(name, name_at, a[1]);
    name[a[1]] = 0;
    bare = strchr(name, ':');
    if (!bare) return refuse(m, "component '%s' has no namespace", name);
    if (kind < BRIDGE_STRUCT || kind > BRIDGE_BITMASK) {
        return refuse(m, "component '%s': its kind is unset", name);
    }
    if (foreign(m, name) && !listed(m->read, m->read_count, name)
        && !listed(m->write, m->write_count, name)) {
        return refuse(m, "it may not declare '%s': that is another namespace's, and no grant "
            "names it", name);
    }
    if (on_unload < BRIDGE_STRIP || on_unload > BRIDGE_DELETE) {
        return refuse(m, "component '%s': on_unload is unset", name);
    }
    print = fingerprint(desc_at, a[6]) ^ kind;

    for (i = 0; i < b->component_count; i ++) {
        if (!strcmp(b->components[i]->name, name)) c = b->components[i];
    }
    if (c && c->fingerprint != print) {
        return refuse(m, "component '%s' has another layout here: fingerprint %016llx, and it "
            "was declared with %016llx", name, (unsigned long long)print,
            (unsigned long long)c->fingerprint);
    }
    if (!c) {
        /* flecs parses the descriptor and computes the layout: Bridge-ASM700 parses no C */
        char *desc = copy(desc_at, a[6]);
        const EcsComponent *layout;
        const char *why = NULL;
        ecs_entity_t id, scope, before;
        int parsed;

        /* A field's type is a bare C name, and flecs looks it up from the scope it is in. So
         * a declared type is named in flecs under its namespace, and a descriptor is parsed
         * from inside that namespace: `Vec3` in game:Body is game:Vec3 - schema.md. */
        *bare = 0;
        scope = ecs_entity(b->world, { .name = name, .parent = b->names });
        *bare = ':';
        id = ecs_entity(b->world, { .name = bare + 1, .parent = scope });
        /* flecs makes an entity an enum when its underlying type is set, which parsing a
         * descriptor does not do. 32 bits, as a C enum is on every target a module has. */
        if (id && kind == BRIDGE_ENUM) {
            /* in place: flecs forbids copying an EcsEnum, so it cannot be set by value */
            ecs_ensure(b->world, id, EcsEnum)->underlying_type = ecs_id(ecs_i32_t);
            ecs_modified(b->world, id, EcsEnum);
        }
        before = ecs_set_scope(b->world, scope);
        parsed = id && !ecs_meta_from_desc(b->world, id, kinds[kind], desc);
        ecs_set_scope(b->world, before);
        layout = parsed ? ecs_get(b->world, id, EcsComponent) : NULL;
        if (layout) why = outside_type_set(b->world, id);
        free(desc);
        if (!layout || why || (uint32_t)layout->size != size || (uint32_t)layout->alignment != align) {
            if (!layout) {
                refuse(m, "component '%s': its descriptor does not describe %s, or names a "
                    "type its namespace has not declared", name, kind_names[kind]);
            } else if (why) {
                refuse(m, "component '%s' holds %s, which no column can", name, why);
            } else {
                refuse(m, "component '%s' is %u bytes aligned to %u in the module and %d "
                    "aligned to %d by its descriptor", name, size, align, layout->size,
                    layout->alignment);
            }
            if (id) ecs_delete(b->world, id);
            return 1;
        }

        c = calloc(1, sizeof *c);
        c->name = copy(name, strlen(name));
        c->id = id;
        c->size = size;
        c->align = align;
        c->fingerprint = print;
        c->zero = calloc(1, size);
        b->components = realloc(b->components, (b->component_count + 1) * sizeof *b->components);
        b->components[b->component_count ++] = c;
    }

    for (i = 0; i < m->declared_count; i ++) {
        if (m->declared[i] == c) {              /* declare is idempotent */
            r[0] = i;
            return 0;
        }
    }
    m->declared = realloc(m->declared, (m->declared_count + 1) * sizeof *m->declared);
    m->declared[m->declared_count] = c;
    c->declarers ++;
    b->layout_moved = 1;
    r[0] = m->declared_count ++;
    return 0;
}

/* flecs says who and what; the observer itself runs at the barrier, after the republish */
static void on_trigger(ecs_iter_t *it)
{
    bridge_system *s = it->ctx;
    uint32_t event = it->event == EcsOnAdd ? BRIDGE_ON_ADD
                   : it->event == EcsOnSet ? BRIDGE_ON_SET : BRIDGE_ON_REMOVE;
    int32_t i;
    for (i = 0; i < it->count; i ++) {
        ecs_entity_t e = it->entities[i];
        if (s->pending_count && s->pending[s->pending_count - 1].id == e) {
            s->pending[s->pending_count - 1].events |= event;
            continue;
        }
        if (s->pending_count == s->pending_room) {
            s->pending_room = s->pending_room ? s->pending_room * 2 : 16;
            s->pending = realloc(s->pending, s->pending_room * sizeof *s->pending);
        }
        s->pending[s->pending_count].id = e;
        s->pending[s->pending_count ++].events = event;
    }
}

/* A system or an observer is a query plus a function, so one registration serves both.
 * `events` is zero for a system, and then `phase` is the phase it named. */
static int subscribe(module *m, uint64_t name_off, uint64_t name_len, uint32_t phase,
                     uint32_t events, uint64_t terms_off, uint64_t term_count, uint64_t *r)
{
    bridge *b = m->b;
    const char *what = events ? "observer" : "system";
    const char *name_at = scratch(m, name_off, name_len);
    const bridge_term *terms = scratch(m, terms_off, term_count * sizeof(bridge_term));
    uint32_t count = (uint32_t)term_count, i;
    ecs_query_desc_t desc = {0};
    bridge_system *s;
    char name[128], full[256];

    if (term_count > BRIDGE_TERMS_MAX) {
        return refuse(m, "a%s %s asks for %llu terms and a query takes %d", events ? "n" : "",
            what, (unsigned long long)term_count, BRIDGE_TERMS_MAX);
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

    s = calloc(1, sizeof *s);
    /* query.md, *What a query may not ask for* */
    for (i = 0; i < count; i ++) {
        static const int16_t access[] = { 0, EcsIn, EcsOut, EcsInOut };
        static const int16_t oper[] = { 0, EcsAnd, EcsNot, EcsOptional };
        bridge_term t;
        component *c;
        memcpy(&t, &terms[i], sizeof t);
        if (t.slot == BRIDGE_WILDCARD) {
            refuse(m, "term %u of %s '%s' is a wildcard", i, what, name);
        } else if (t.oper == BRIDGE_OR) {
            refuse(m, "term %u of %s '%s' is an or-term", i, what, name);
        } else if (t.slot >= m->declared_count) {
            refuse(m, "term %u of %s '%s' names slot %u, which it never declared", i, what, name,
                t.slot);
        } else if (t.access < BRIDGE_IN || t.access > BRIDGE_INOUT) {
            refuse(m, "term %u of %s '%s': its access is unset", i, what, name);
        } else if (t.oper < BRIDGE_AND || t.oper > BRIDGE_OPTIONAL) {
            refuse(m, "term %u of %s '%s': its operator is unset", i, what, name);
        } else if (t.access != BRIDGE_IN && !may_write(m, m->declared[t.slot])) {
            refuse(m, "%s '%s' may not write '%s': no grant says so", what, name,
                m->declared[t.slot]->name);
        }
        if (m->refused) {
            free(s);
            return 1;
        }
        c = m->declared[t.slot];
        desc.terms[i].id = c->id;
        desc.terms[i].inout = access[t.access];
        desc.terms[i].oper = oper[t.oper];
        s->sizes[i] = c->size;
        /* what the conflict graph reads: a term that must not match touches nothing */
        s->touches[i] = t.oper == BRIDGE_NOT ? NULL : c;
        s->writes[i] = t.access != BRIDGE_IN;
    }

    if (events) {
        ecs_observer_desc_t od = {0};
        uint32_t k = 0;
        memcpy(od.query.terms, desc.terms, sizeof desc.terms);
        if (events & BRIDGE_ON_ADD) od.events[k ++] = EcsOnAdd;
        if (events & BRIDGE_ON_SET) od.events[k ++] = EcsOnSet;
        if (events & BRIDGE_ON_REMOVE) od.events[k ++] = EcsOnRemove;
        od.callback = on_trigger;
        od.ctx = s;
        s->observing = ecs_observer_init(b->world, &od);
    } else if (count) {
        /* flecs fails creation unless every term matches whole tables, which one span per
         * table assumes; what is left needs a read per row */
        desc.cache_kind = EcsQueryCacheAll;
        s->query = ecs_query_init(b->world, &desc);
        if (s->query && s->query->row_fields) {
            ecs_query_fini(s->query);
            s->query = NULL;
        }
    }
    if (events ? !s->observing : count && !s->query) {
        free(s);
        return refuse(m, "%s '%s' asks for something flecs refuses, or a term with no "
            "base-and-stride form", what, name);
    }

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

/* Nothing below touches flecs. Modules run at once, and what one asks for must not depend on
 * who ran beside it - schedule.md, *What a pass depends on*. So an id comes from a block set
 * aside for the module, arena space from its own share, and a change goes on its own queue,
 * which the barrier applies in an order that never varies. No lock: a module's systems are
 * serialised, so each of the three has one writer. */

static void queue(module *m, uint32_t op, ecs_entity_t id, component *c, const void *value)
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
static int abi_spawn(void *ud, const uint64_t *a, uint64_t *r)
{
    module *m = ud;
    (void)a;
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
static int abi_destroy(void *ud, const uint64_t *a, uint64_t *r)
{
    (void)r;
    queue(ud, CHANGE_DESTROY, a[0], NULL, NULL);
    return 0;
}

static component *written(module *m, uint64_t slot, const char *entry)
{
    if (slot >= m->declared_count) {
        refuse(m, "%s names slot %llu, which it never declared", entry, (unsigned long long)slot);
        return NULL;
    }
    if (!may_write(m, m->declared[slot])) {
        refuse(m, "it may not %s '%s': no grant says so", entry, m->declared[slot]->name);
        return NULL;
    }
    return m->declared[slot];
}

/* attach(id, slot, ptr, len). The value travels with it; zero length means zeroed. */
static int abi_attach(void *ud, const uint64_t *a, uint64_t *r)
{
    module *m = ud;
    component *c = written(m, a[1], "attach");
    const void *value;
    (void)r;
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
static int abi_detach(void *ud, const uint64_t *a, uint64_t *r)
{
    module *m = ud;
    component *c = written(m, a[1], "detach");
    (void)r;
    if (!c) return 1;
    queue(m, CHANGE_DETACH, a[0], c, NULL);
    return 0;
}

/* alloc(len) -> offset into the persistent arena. Never reclaimed - strings.md. */
static int abi_alloc(void *ud, const uint64_t *a, uint64_t *r)
{
    module *m = ud;
    uint64_t len = (a[0] + 7u) & ~7ull;
    if (m->persistent <= 0) {
        return refuse(m, "alloc: it stated that it keeps nothing in the persistent arena");
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
    enum { I = BACKEND_I32, L = BACKEND_I64 };
    static const struct {
        const char *name;
        backend_fn  fn;
        uint8_t     params[8];
        uint32_t    param_count;
        uint8_t     results[1];
        uint32_t    result_count;
    } entries[] = {
        { "declare", abi_declare, { I, I, I, I, I, I, I, I }, 8, { I }, 1 },
        { "system",  abi_system,  { I, I, I, I, I, I },    6, { I }, 1 },
        { "observe", abi_observe, { I, I, I, I, I },       5, { I }, 1 },
        { "spawn",   abi_spawn,   { 0 },                   0, { L }, 1 },
        { "destroy", abi_destroy, { L },                   1, { 0 }, 0 },
        { "attach",  abi_attach,  { L, I, I, I },          4, { 0 }, 0 },
        { "detach",  abi_detach,  { L, I },                2, { 0 }, 0 },
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
