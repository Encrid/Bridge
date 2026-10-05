/* Storage, exercised and told about: every structural change a module can ask for, each in a
 * phase of its own so that a host can make one and then look, and what a system and an observer
 * see afterwards. It says what it saw in its log and decides nothing.
 *
 * Built three ways. Plain: a handful of entities, with observers, for what storage does. BIG:
 * many entities and no observer, for what it costs. TABLES: ten thousand tables.
 * docs/plan/PHASE10.md. */
#include "bridge-native.h"

ECS_STRUCT(A, { uint32_t v; });
ECS_STRUCT(B, { uint32_t v; });

#ifdef BIG
constexpr uint32_t COUNT = 100'000;
#elifdef TABLES
constexpr uint32_t COUNT = 10'000;
constexpr uint32_t KINDS = 14;                  /* 2^14 sets of them, each a table */
ECS_STRUCT(T, { uint32_t v; });
#else
constexpr uint32_t COUNT = 6;
#endif

static const bridge_api *api;
static uint32_t a, b;
static uint64_t ids[COUNT];

BRIDGE_NATIVE_BIND(api)

[[maybe_unused]] static void say(const char *text, uint32_t x, uint32_t y)
{
    bridge_native_log(api, 0, text, (uint32_t)strlen(text), x, y);
}

/* ---- the changes: one phase each ---- */

BRIDGE_NATIVE(spawn)
{
    for (uint32_t i = 0; i < COUNT; i ++) ids[i] = api->spawn();
    return 0;
}

/* zero bytes, so that a hundred thousand of them need no scratch arena */
BRIDGE_NATIVE(attach_a) { for (uint32_t i = 0; i < COUNT; i ++) api->attach(ids[i], a, 0, 0); return 0; }
BRIDGE_NATIVE(attach_b) { for (uint32_t i = 0; i < COUNT; i ++) api->attach(ids[i], b, 0, 0); return 0; }
BRIDGE_NATIVE(detach_b) { for (uint32_t i = 0; i < COUNT; i ++) api->detach(ids[i], b); return 0; }
BRIDGE_NATIVE(destroy) { for (uint32_t i = 0; i < COUNT; i ++) api->destroy(ids[i]); return 0; }

/* two components in one barrier, on entities that carry neither */
BRIDGE_NATIVE(attach_both)
{
    for (uint32_t i = 0; i < COUNT; i ++) {
        api->attach(ids[i], a, 0, 0);
        api->attach(ids[i], b, 0, 0);
    }
    return 0;
}

#ifdef TABLES
static uint32_t kinds[KINDS], seeing;

/* entity i carries the kinds whose bits are set in i + 1: no two carry the same set */
BRIDGE_NATIVE(combine)
{
    for (uint32_t i = 0; i < COUNT; i ++) {
        for (uint32_t k = 0; k < KINDS; k ++) {
            if ((i + 1) >> k & 1) api->attach(ids[i], kinds[k], 0, 0);
        }
    }
    return 0;
}

BRIDGE_NATIVE(see)
{
    const bridge_query *q = bridge_native_query(api, seeing);
    uint32_t rows = 0;
    for (uint32_t s = 0; s < q->spans; s ++) rows += bridge_native_span(api, q, s, 1)[0];
    say("the first kind: spans, rows", q->spans, rows);
    return 0;
}
#endif

#if !defined(BIG) && !defined(TABLES)
static uint32_t with_a, without_b, maybe_b, with_b;
static uint64_t old[COUNT], gone[64];
static uint32_t gone_count, reused;

/* where an id sits among this module's own, so that nothing printed depends on how ids are
 * numbered: a current one, then one it destroyed, then neither */
static uint32_t which(uint64_t id)
{
    for (uint32_t i = 0; i < COUNT; i ++) if (ids[i] == id) return i;
    for (uint32_t i = 0; i < COUNT; i ++) if (old[i] == id) return 100 + i;
    return 999;     /* one destroyed longer ago, or never this module's */
}

static bool alive(uint64_t id)
{
    const bridge_header *h = bridge_native_header(api);
    auto index = (const bridge_entity *)(api->shared_base + h->entity_index);
    return (uint32_t)id < h->entities && index[(uint32_t)id].generation == (uint32_t)(id >> 32);
}

/* half of them, and one of them */
BRIDGE_NATIVE(attach_b_half) { for (uint32_t i = 0; i < COUNT / 2; i ++) api->attach(ids[i], b, 0, 0); return 0; }
BRIDGE_NATIVE(detach_b_half) { for (uint32_t i = 0; i < COUNT / 2; i ++) api->detach(ids[i], b); return 0; }
BRIDGE_NATIVE(detach_b_absent) { api->detach(ids[COUNT - 1], b); return 0; }

/* a value, to see that it is the bytes at the call that are kept */
BRIDGE_NATIVE(value_a)
{
    A value = { 41 };
    uint32_t at = bridge_native_put(api, &value, sizeof value);
    api->attach(ids[0], a, at, sizeof value);
    ((A *)(api->private_base + at))->v = 99;        /* after the call: must not be seen */
    return 0;
}

/* on one entity inside one barrier: there and gone, and gone and back */
BRIDGE_NATIVE(flicker)
{
    api->attach(ids[COUNT - 1], b, 0, 0);
    api->detach(ids[COUNT - 1], b);
    api->detach(ids[0], a);
    api->attach(ids[0], a, 0, 0);
    return 0;
}

/* the second half is destroyed, twice, and then asked for again */
BRIDGE_NATIVE(destroy_half)
{
    for (uint32_t i = COUNT / 2; i < COUNT; i ++) {
        old[i] = ids[i];
        if (gone_count < 64) gone[gone_count ++] = ids[i];
        api->destroy(ids[i]);
        api->destroy(ids[i]);
    }
    return 0;
}

BRIDGE_NATIVE(respawn_half)
{
    uint32_t same = 0, dead = 0;
    for (uint32_t i = COUNT / 2; i < COUNT; i ++) {
        ids[i] = api->spawn();
        /* an index a destroyed id had is one of ours again: never under the same generation */
        for (uint32_t k = 0; k < gone_count; k ++) {
            if ((uint32_t)ids[i] != (uint32_t)gone[k]) continue;
            reused = 1;
            if (ids[i] == gone[k]) same ++;
        }
    }
    for (uint32_t k = 0; k < gone_count; k ++) dead += !alive(gone[k]);
    say("destroyed ids that read as dead, of", dead, gone_count);
    say("ids that came back whole", same, 0);
    return 0;
}

/* when a freed index comes back is storage's own business; that one does, in the end, is not */
BRIDGE_NATIVE(recycled)
{
    say("an index was used again", reused, 0);
    return 0;
}

/* a change asked of an id that is gone is passed over */
BRIDGE_NATIVE(touch_dead)
{
    api->attach(old[COUNT - 1], a, 0, 0);
    api->detach(old[COUNT - 1], a);
    api->destroy(old[COUNT - 1]);
    return 0;
}

BRIDGE_NATIVE(attach_a_half) { for (uint32_t i = COUNT / 2; i < COUNT; i ++) api->attach(ids[i], a, 0, 0); return 0; }

/* ---- what a system sees ---- */

BRIDGE_NATIVE(look)
{
    const bridge_query *q = bridge_native_query(api, with_a);
    uint32_t rows = 0, empty = 0, s;
    for (s = 0; s < q->spans; s ++) {
        const uint32_t *span = bridge_native_span(api, q, s, 1);
        rows += span[0];
        empty += !span[0];
        if (!span[2]) say("FAIL: a matched term with no base", s, 0);
    }
    say("with A: spans, rows", q->spans, rows);
    if (empty) say("with A: spans of no rows", empty, 0);
    if (rows) {
        const uint32_t *span = bridge_native_span(api, q, 0, 1);
        for (s = 0; !span[0]; s ++) span = bridge_native_span(api, q, s + 1, 1);
        say("first A", ((const A *)(api->shared_base + span[2]))->v, 0);
    }

    q = bridge_native_query(api, without_b);
    for (rows = 0, s = 0; s < q->spans; s ++) {
        const uint32_t *span = bridge_native_span(api, q, s, 2);
        rows += span[0];
        if (span[3]) say("FAIL: a NOT term with a base", s, 0);
    }
    say("with A and not B: spans, rows", q->spans, rows);

    q = bridge_native_query(api, maybe_b);
    uint32_t have = 0;
    for (rows = 0, s = 0; s < q->spans; s ++) {
        const uint32_t *span = bridge_native_span(api, q, s, 2);
        rows += span[0];
        if (span[3]) have += span[0];
    }
    say("with A and maybe B: spans, rows", q->spans, rows);
    say("of them with B: rows", have, 0);
    return 0;
}

/* What storage publishes in the shared block, written over by a module that should not: the
 * index record of every one of its entities, and the entity array of every table with A. Each
 * is made to name the first entity, which is alive: a storage that read either back would act
 * on it. */
BRIDGE_NATIVE(scribble)
{
    const bridge_header *h = bridge_native_header(api);
    auto index = (bridge_entity *)(api->shared_base + h->entity_index);
    const bridge_query *q = bridge_native_query(api, with_a);
    bridge_entity first = index[(uint32_t)ids[0]];
    for (uint32_t i = 0; i < COUNT; i ++) index[(uint32_t)ids[i]] = first;
    for (uint32_t s = 0; s < q->spans; s ++) {
        const uint32_t *span = bridge_native_span(api, q, s, 1);
        auto who = (uint64_t *)(api->shared_base + span[1]);
        for (uint32_t row = 0; row < span[0]; row ++) who[row] = ids[0];
    }
    return 0;
}

/* each entity that is left, by its own id: where the index says it is, and what is there */
BRIDGE_NATIVE(find)
{
    const bridge_header *h = bridge_native_header(api);
    auto index = (const bridge_entity *)(api->shared_base + h->entity_index);
    auto base = (const uint32_t *)(api->private_base + h->column_base);
    uint32_t found = 0, named = 0;
    for (uint32_t i = 0; i < COUNT; i ++) {
        const bridge_entity *e = &index[(uint32_t)ids[i]];
        const bridge_query *q = bridge_native_query(api, with_a);
        if (e->generation != (uint32_t)(ids[i] >> 32)) continue;
        found += base[e->table * h->terms + a] != 0;
        for (uint32_t s = 0; s < q->spans; s ++) {
            const uint32_t *span = bridge_native_span(api, q, s, 1);
            auto who = (const uint64_t *)(api->shared_base + span[1]);
            for (uint32_t row = 0; row < span[0]; row ++) named += who[row] == ids[i];
        }
    }
    say("alive and found through the index with A, and named in a span", found, named);
    return 0;
}

/* B on the first half, each with a value of its own, and what a query for B alone then sees:
 * for a column that has to be where it was after the one before it is taken away */
BRIDGE_NATIVE(value_b)
{
    for (uint32_t i = 0; i < COUNT / 2; i ++) {
        B value = { 70 + i };
        api->attach(ids[i], b, bridge_native_put(api, &value, sizeof value), sizeof value);
    }
    return 0;
}

BRIDGE_NATIVE(look_b)
{
    const bridge_query *q = bridge_native_query(api, with_b);
    uint32_t rows = 0, sum = 0;
    for (uint32_t s = 0; s < q->spans; s ++) {
        const uint32_t *span = bridge_native_span(api, q, s, 1);
        auto values = (const B *)(api->shared_base + span[2]);
        rows += span[0];
        for (uint32_t row = 0; row < span[0]; row ++) sum += values[row].v;
    }
    say("with B: spans, rows", q->spans, rows);
    say("with B: the sum of them", sum, 0);
    return 0;
}

/* ---- what an observer hears: who, and how ---- */

static uint32_t on_a, on_b, on_both;

static int heard(const char *what, uint32_t observer)
{
    const bridge_list *list = bridge_native_list(api, observer);
    auto t = (const bridge_trigger *)(api->private_base + list->at);
    for (uint32_t i = 0; i < list->count; i ++) say(what, which(t[i].id), t[i].events);
    return 0;
}

BRIDGE_NATIVE(heard_a) { return heard("A: who, events", on_a); }
BRIDGE_NATIVE(heard_b) { return heard("B: who, events", on_b); }
BRIDGE_NATIVE(heard_both) { return heard("A and B: who, events", on_both); }
#endif

BRIDGE_NATIVE(init)
{
    int none = 0;
    a = BRIDGE_NATIVE_COMPONENT(api, "st:A", A, BRIDGE_DELETE);
    b = BRIDGE_NATIVE_COMPONENT(api, "st:B", B, BRIDGE_DELETE);
    BRIDGE_NATIVE_SYSTEM(api, "spawn", "spawn", &none, 0);
    BRIDGE_NATIVE_SYSTEM(api, "attach_a", "attach_a", &none, 0);
    BRIDGE_NATIVE_SYSTEM(api, "attach_b", "attach_b", &none, 0);
    BRIDGE_NATIVE_SYSTEM(api, "detach_b", "detach_b", &none, 0);
    BRIDGE_NATIVE_SYSTEM(api, "destroy", "destroy", &none, 0);
    BRIDGE_NATIVE_SYSTEM(api, "attach_both", "attach_both", &none, 0);
#ifdef TABLES
    static const char *const names[KINDS] = {
        "st:T0", "st:T1", "st:T2", "st:T3", "st:T4", "st:T5", "st:T6",
        "st:T7", "st:T8", "st:T9", "st:T10", "st:T11", "st:T12", "st:T13"
    };
    bridge_term first[1] = {{ 0, BRIDGE_IN, BRIDGE_AND, 0 }};
    for (uint32_t k = 0; k < KINDS; k ++) {
        kinds[k] = api->declare(bridge_native_put(api, names[k], (uint32_t)strlen(names[k])),
            (uint32_t)strlen(names[k]), BRIDGE_STRUCT, sizeof(T), alignof(T),
            bridge_native_put(api, T_desc, sizeof T_desc - 1), sizeof T_desc - 1, BRIDGE_DELETE);
    }
    first[0].slot = kinds[0];
    BRIDGE_NATIVE_SYSTEM(api, "combine", "combine", &none, 0);
    seeing = BRIDGE_NATIVE_SYSTEM(api, "see", "look", first, 1);
#endif
#if !defined(BIG) && !defined(TABLES)
    bridge_term one[1] = {{ a, BRIDGE_IN, BRIDGE_AND, 0 }};
    bridge_term two[2] = {{ a, BRIDGE_IN, BRIDGE_AND, 0 }, { b, BRIDGE_IN, BRIDGE_NOT, 0 }};
    BRIDGE_NATIVE_SYSTEM(api, "attach_b_half", "attach_b_half", &none, 0);
    BRIDGE_NATIVE_SYSTEM(api, "detach_b_half", "detach_b_half", &none, 0);
    BRIDGE_NATIVE_SYSTEM(api, "detach_b_absent", "detach_b_absent", &none, 0);
    BRIDGE_NATIVE_SYSTEM(api, "value_a", "value_a", &none, 0);
    BRIDGE_NATIVE_SYSTEM(api, "flicker", "flicker", &none, 0);
    BRIDGE_NATIVE_SYSTEM(api, "destroy_half", "destroy_half", &none, 0);
    BRIDGE_NATIVE_SYSTEM(api, "respawn_half", "respawn_half", &none, 0);
    BRIDGE_NATIVE_SYSTEM(api, "touch_dead", "touch_dead", &none, 0);
    BRIDGE_NATIVE_SYSTEM(api, "recycled", "recycled", &none, 0);
    BRIDGE_NATIVE_SYSTEM(api, "attach_a_half", "attach_a_half", &none, 0);
    BRIDGE_NATIVE_SYSTEM(api, "scribble", "scribble", &none, 0);
    BRIDGE_NATIVE_SYSTEM(api, "find", "find", &none, 0);
    BRIDGE_NATIVE_SYSTEM(api, "value_b", "value_b", &none, 0);
    with_a = BRIDGE_NATIVE_SYSTEM(api, "look", "look", one, 1);
    without_b = BRIDGE_NATIVE_SYSTEM(api, "look_not", "never", two, 2);
    two[1].oper = BRIDGE_OPTIONAL;
    maybe_b = BRIDGE_NATIVE_SYSTEM(api, "look_maybe", "never", two, 2);
    on_a = BRIDGE_NATIVE_OBSERVER(api, "heard_a",
        BRIDGE_ON_ADD | BRIDGE_ON_SET | BRIDGE_ON_REMOVE, one, 1);
    one[0].slot = b;
    with_b = BRIDGE_NATIVE_SYSTEM(api, "look_b", "look_b", one, 1);
    on_b = BRIDGE_NATIVE_OBSERVER(api, "heard_b",
        BRIDGE_ON_ADD | BRIDGE_ON_SET | BRIDGE_ON_REMOVE, one, 1);
    two[1].oper = BRIDGE_AND;
    on_both = BRIDGE_NATIVE_OBSERVER(api, "heard_both",
        BRIDGE_ON_ADD | BRIDGE_ON_SET | BRIDGE_ON_REMOVE, two, 2);
#endif
    return 0;
}

#if !defined(BIG) && !defined(TABLES)
/* registered in a phase no host runs: they exist for the spans their queries are given */
BRIDGE_NATIVE(look_not) { return 0; }
BRIDGE_NATIVE(look_maybe) { return 0; }
#endif

BRIDGE_NATIVE(shutdown) { return 0; }
BRIDGE_NATIVE(reload) { return 0; }
