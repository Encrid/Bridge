/* Storage, exercised and told about: every structural change a module can ask for, each in a
 * phase of its own so that a host can make one and then look, and what a system and an observer
 * see afterwards. It says what it saw in its log and decides nothing.
 *
 * Built three ways. Plain: a handful of entities, with observers, for what storage does. BIG:
 * many entities and no observer, for what it costs. TABLES: ten thousand tables. And BIG again
 * with A made larger, FAT64 to FAT4K.
 * docs/plan/STAGE10.md. */
#include "bridge-native.h"

/* A is as big as the build says: for what a change costs an entity that carries more. The
 * length is written out each time, since a descriptor is the text as it was typed. */
#if defined(FAT64)
BRIDGE_SCHEMA_STRUCT(A, { uint32_t v; char rest[60]; });
#elif defined(FAT256)
BRIDGE_SCHEMA_STRUCT(A, { uint32_t v; char rest[252]; });
#elif defined(FAT1K)
BRIDGE_SCHEMA_STRUCT(A, { uint32_t v; char rest[1020]; });
#elif defined(FAT4K)
BRIDGE_SCHEMA_STRUCT(A, { uint32_t v; char rest[4092]; });
#else
BRIDGE_SCHEMA_STRUCT(A, { uint32_t v; });
#endif
BRIDGE_SCHEMA_STRUCT(B, { uint32_t v; });

#if defined(FAT1K) || defined(FAT4K)
constexpr uint32_t COUNT = 10'000;
#elifdef BIG
constexpr uint32_t COUNT = 100'000;
#elifdef TABLES
constexpr uint32_t COUNT = 10'000;
constexpr uint32_t KINDS = 14;                  /* 2^14 sets of them, each a table */
BRIDGE_SCHEMA_STRUCT(T, { uint32_t v; });
#else
constexpr uint32_t COUNT = 6;
#endif

static uint32_t a, b;
static uint64_t ids[COUNT];


[[maybe_unused]] static void say(const bridge_api_t *api, const char *text, uint32_t x, uint32_t y)
{
    bridge_native_log(api, BRIDGE_LOG_INFO, text, (uint32_t)strlen(text), x, y);
}

/* ---- the changes: one phase each ---- */

BRIDGE_NATIVE(spawn)
{
    for (uint32_t i = 0; i < COUNT; i ++) ids[i] = api->spawn(api);
    return 0;
}

/* zero bytes, so that a hundred thousand of them need no scratch arena */
BRIDGE_NATIVE(attach_a) { for (uint32_t i = 0; i < COUNT; i ++) api->attach(api, ids[i], a, 0, 0); return 0; }
BRIDGE_NATIVE(attach_b) { for (uint32_t i = 0; i < COUNT; i ++) api->attach(api, ids[i], b, 0, 0); return 0; }
BRIDGE_NATIVE(detach_b) { for (uint32_t i = 0; i < COUNT; i ++) api->detach(api, ids[i], b); return 0; }
BRIDGE_NATIVE(destroy) { for (uint32_t i = 0; i < COUNT; i ++) api->destroy(api, ids[i]); return 0; }

/* two components in one barrier, on entities that carry neither */
BRIDGE_NATIVE(attach_both)
{
    for (uint32_t i = 0; i < COUNT; i ++) {
        api->attach(api, ids[i], a, 0, 0);
        api->attach(api, ids[i], b, 0, 0);
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
            if ((i + 1) >> k & 1) api->attach(api, ids[i], kinds[k], 0, 0);
        }
    }
    return 0;
}

BRIDGE_NATIVE(see)
{
    const bridge_query_t *q = bridge_native_query(api, seeing);
    uint32_t rows = 0;
    for (uint32_t s = 0; s < q->spans; s ++) rows += bridge_native_span(api, q, s, 1)[0];
    say(api, "the first kind: spans, rows", q->spans, rows);
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

static bool alive(const bridge_api_t *api, uint64_t id)
{
    const bridge_header_t *h = bridge_native_header(api);
    auto index = (const bridge_entity_t *)(api->shared_base + h->entity_index);
    return (uint32_t)id < h->entities && index[(uint32_t)id].generation == (uint32_t)(id >> 32);
}

/* half of them, and one of them */
BRIDGE_NATIVE(attach_b_half) { for (uint32_t i = 0; i < COUNT / 2; i ++) api->attach(api, ids[i], b, 0, 0); return 0; }
BRIDGE_NATIVE(detach_b_half) { for (uint32_t i = 0; i < COUNT / 2; i ++) api->detach(api, ids[i], b); return 0; }
BRIDGE_NATIVE(detach_b_absent) { api->detach(api, ids[COUNT - 1], b); return 0; }

/* a value, to see that it is the bytes at the call that are kept */
BRIDGE_NATIVE(value_a)
{
    A value = { 41 };
    uint32_t at = bridge_native_put(api, &value, sizeof value);
    api->attach(api, ids[0], a, at, sizeof value);
    ((A *)(api->private_base + at))->v = 99;        /* after the call: must not be seen */
    return 0;
}

/* on one entity inside one barrier: there and gone, and gone and back */
BRIDGE_NATIVE(flicker)
{
    api->attach(api, ids[COUNT - 1], b, 0, 0);
    api->detach(api, ids[COUNT - 1], b);
    api->detach(api, ids[0], a);
    api->attach(api, ids[0], a, 0, 0);
    return 0;
}

/* the second half is destroyed, twice, and then asked for again */
BRIDGE_NATIVE(destroy_half)
{
    for (uint32_t i = COUNT / 2; i < COUNT; i ++) {
        old[i] = ids[i];
        if (gone_count < 64) gone[gone_count ++] = ids[i];
        api->destroy(api, ids[i]);
        api->destroy(api, ids[i]);
    }
    return 0;
}

BRIDGE_NATIVE(respawn_half)
{
    uint32_t same = 0, dead = 0;
    for (uint32_t i = COUNT / 2; i < COUNT; i ++) {
        ids[i] = api->spawn(api);
        /* an index a destroyed id had is one of ours again: never under the same generation */
        for (uint32_t k = 0; k < gone_count; k ++) {
            if ((uint32_t)ids[i] != (uint32_t)gone[k]) continue;
            reused = 1;
            if (ids[i] == gone[k]) same ++;
        }
    }
    for (uint32_t k = 0; k < gone_count; k ++) dead += !alive(api, gone[k]);
    say(api, "destroyed ids that read as dead, of", dead, gone_count);
    say(api, "ids that came back whole", same, 0);
    return 0;
}

/* when a freed index comes back is storage's own business; that one does, in the end, is not */
BRIDGE_NATIVE(recycled)
{
    say(api, "an index was used again", reused, 0);
    return 0;
}

/* a change asked of an id that is gone is passed over */
BRIDGE_NATIVE(touch_dead)
{
    api->attach(api, old[COUNT - 1], a, 0, 0);
    api->detach(api, old[COUNT - 1], a);
    api->destroy(api, old[COUNT - 1]);
    return 0;
}

BRIDGE_NATIVE(attach_a_half) { for (uint32_t i = COUNT / 2; i < COUNT; i ++) api->attach(api, ids[i], a, 0, 0); return 0; }

/* ---- what a system sees ---- */

BRIDGE_NATIVE(look)
{
    const bridge_query_t *q = bridge_native_query(api, with_a);
    uint32_t rows = 0, empty = 0, s;
    for (s = 0; s < q->spans; s ++) {
        const uint32_t *span = bridge_native_span(api, q, s, 1);
        rows += span[0];
        empty += !span[0];
        if (!span[2]) say(api, "FAIL: a matched term with no base", s, 0);
    }
    say(api, "with A: spans, rows", q->spans, rows);
    if (empty) say(api, "with A: spans of no rows", empty, 0);
    if (rows) {
        const uint32_t *span = bridge_native_span(api, q, 0, 1);
        for (s = 0; !span[0]; s ++) span = bridge_native_span(api, q, s + 1, 1);
        say(api, "first A", ((const A *)(api->shared_base + span[2]))->v, 0);
    }

    q = bridge_native_query(api, without_b);
    for (rows = 0, s = 0; s < q->spans; s ++) {
        const uint32_t *span = bridge_native_span(api, q, s, 2);
        rows += span[0];
        if (span[3]) say(api, "FAIL: a NOT term with a base", s, 0);
    }
    say(api, "with A and not B: spans, rows", q->spans, rows);

    q = bridge_native_query(api, maybe_b);
    uint32_t have = 0;
    for (rows = 0, s = 0; s < q->spans; s ++) {
        const uint32_t *span = bridge_native_span(api, q, s, 2);
        rows += span[0];
        if (span[3]) have += span[0];
    }
    say(api, "with A and maybe B: spans, rows", q->spans, rows);
    say(api, "of them with B: rows", have, 0);
    return 0;
}

/* What storage publishes in the shared block, written over by a module that should not: the
 * index record of every one of its entities, and the entity array of every table with A. Each
 * is made to name the first entity, which is alive: a storage that read either back would act
 * on it. */
BRIDGE_NATIVE(scribble)
{
    const bridge_header_t *h = bridge_native_header(api);
    auto index = (bridge_entity_t *)(api->shared_base + h->entity_index);
    const bridge_query_t *q = bridge_native_query(api, with_a);
    bridge_entity_t first = index[(uint32_t)ids[0]];
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
    const bridge_header_t *h = bridge_native_header(api);
    auto index = (const bridge_entity_t *)(api->shared_base + h->entity_index);
    auto base = (const uint32_t *)(api->private_base + h->column_base);
    uint32_t found = 0, named = 0;
    for (uint32_t i = 0; i < COUNT; i ++) {
        const bridge_entity_t *e = &index[(uint32_t)ids[i]];
        const bridge_query_t *q = bridge_native_query(api, with_a);
        if (e->generation != (uint32_t)(ids[i] >> 32)) continue;
        found += base[e->table * h->types + a] != 0;
        for (uint32_t s = 0; s < q->spans; s ++) {
            const uint32_t *span = bridge_native_span(api, q, s, 1);
            auto who = (const uint64_t *)(api->shared_base + span[1]);
            for (uint32_t row = 0; row < span[0]; row ++) named += who[row] == ids[i];
        }
    }
    say(api, "alive and found through the index with A, and named in a span", found, named);
    return 0;
}

/* B on the first half, each with a value of its own, and what a query for B alone then sees:
 * for a column that has to be where it was after the one before it is taken away */
BRIDGE_NATIVE(value_b)
{
    for (uint32_t i = 0; i < COUNT / 2; i ++) {
        B value = { 70 + i };
        bridge_native_attach(api, ids[i], b, &value, sizeof value);
    }
    return 0;
}

BRIDGE_NATIVE(look_b)
{
    const bridge_query_t *q = bridge_native_query(api, with_b);
    uint32_t rows = 0, sum = 0;
    for (uint32_t s = 0; s < q->spans; s ++) {
        const uint32_t *span = bridge_native_span(api, q, s, 1);
        auto values = (const B *)(api->shared_base + span[2]);
        rows += span[0];
        for (uint32_t row = 0; row < span[0]; row ++) sum += values[row].v;
    }
    say(api, "with B: spans, rows", q->spans, rows);
    say(api, "with B: the sum of them", sum, 0);
    return 0;
}

/* ---- what an observer hears: who, and how ---- */

static uint32_t on_a, on_b, on_both, on_alone;

static int heard(const bridge_api_t *api, const char *what, uint32_t observer)
{
    const bridge_list_t *list = bridge_native_list(api, observer);
    auto t = (const bridge_trigger_t *)(api->private_base + list->at);
    for (uint32_t i = 0; i < list->count; i ++) say(api, what, which(t[i].id), t[i].events);
    return 0;
}

BRIDGE_NATIVE(heard_a) { return heard(api, "A: who, events", on_a); }
BRIDGE_NATIVE(heard_b) { return heard(api, "B: who, events", on_b); }
BRIDGE_NATIVE(heard_both) { return heard(api, "A and B: who, events", on_both); }
BRIDGE_NATIVE(heard_alone) { return heard(api, "A and not B: who, events", on_alone); }
#endif

BRIDGE_NATIVE(init)
{
    int none = 0;
    a = BRIDGE_NATIVE_COMPONENT(api, "st:A", A, BRIDGE_DELETE, BRIDGE_TABLE);
    b = BRIDGE_NATIVE_COMPONENT(api, "st:B", B, BRIDGE_DELETE, BRIDGE_TABLE);
    /* what a system attaches or detaches is stated with it */
    uint32_t both[2] = { a, b };
    BRIDGE_NATIVE_SYSTEM(api, "spawn", "spawn", &none, 0, nullptr, 0);
    BRIDGE_NATIVE_SYSTEM(api, "attach_a", "attach_a", &none, 0, &a, 1);
    BRIDGE_NATIVE_SYSTEM(api, "attach_b", "attach_b", &none, 0, &b, 1);
    BRIDGE_NATIVE_SYSTEM(api, "detach_b", "detach_b", &none, 0, &b, 1);
    BRIDGE_NATIVE_SYSTEM(api, "destroy", "destroy", &none, 0, nullptr, 0);
    BRIDGE_NATIVE_SYSTEM(api, "attach_both", "attach_both", &none, 0, both, 2);
#ifdef TABLES
    static const char *const names[KINDS] = {
        "st:T0", "st:T1", "st:T2", "st:T3", "st:T4", "st:T5", "st:T6",
        "st:T7", "st:T8", "st:T9", "st:T10", "st:T11", "st:T12", "st:T13"
    };
    bridge_term_t first[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    for (uint32_t k = 0; k < KINDS; k ++) {
        kinds[k] = api->declare(api, bridge_native_put(api, names[k], (uint32_t)strlen(names[k])),
            (uint32_t)strlen(names[k]), BRIDGE_STRUCT, sizeof(T), alignof(T),
            bridge_native_put(api, T_desc, sizeof T_desc - 1), sizeof T_desc - 1, BRIDGE_DELETE, BRIDGE_TABLE);
    }
    first[0].slot = kinds[0];
    BRIDGE_NATIVE_SYSTEM(api, "combine", "combine", &none, 0, kinds, 14);
    seeing = BRIDGE_NATIVE_SYSTEM(api, "see", "look", first, 1, nullptr, 0);
#endif
#if !defined(BIG) && !defined(TABLES)
    bridge_term_t one[1] = {{ a, BRIDGE_READ, BRIDGE_AND, 0 }};
    bridge_term_t two[2] = {{ a, BRIDGE_READ, BRIDGE_AND, 0 }, { b, BRIDGE_READ, BRIDGE_NOT, 0 }};
    BRIDGE_NATIVE_SYSTEM(api, "attach_b_half", "attach_b_half", &none, 0, &b, 1);
    BRIDGE_NATIVE_SYSTEM(api, "detach_b_half", "detach_b_half", &none, 0, &b, 1);
    BRIDGE_NATIVE_SYSTEM(api, "detach_b_absent", "detach_b_absent", &none, 0, &b, 1);
    BRIDGE_NATIVE_SYSTEM(api, "value_a", "value_a", &none, 0, &a, 1);
    BRIDGE_NATIVE_SYSTEM(api, "flicker", "flicker", &none, 0, both, 2);
    BRIDGE_NATIVE_SYSTEM(api, "destroy_half", "destroy_half", &none, 0, nullptr, 0);
    BRIDGE_NATIVE_SYSTEM(api, "respawn_half", "respawn_half", &none, 0, nullptr, 0);
    BRIDGE_NATIVE_SYSTEM(api, "touch_dead", "touch_dead", &none, 0, &a, 1);
    BRIDGE_NATIVE_SYSTEM(api, "recycled", "recycled", &none, 0, nullptr, 0);
    BRIDGE_NATIVE_SYSTEM(api, "attach_a_half", "attach_a_half", &none, 0, &a, 1);
    BRIDGE_NATIVE_SYSTEM(api, "scribble", "scribble", &none, 0, nullptr, 0);
    BRIDGE_NATIVE_SYSTEM(api, "find", "find", &none, 0, nullptr, 0);
    BRIDGE_NATIVE_SYSTEM(api, "value_b", "value_b", &none, 0, &b, 1);
    with_a = BRIDGE_NATIVE_SYSTEM(api, "look", "look", one, 1, nullptr, 0);
    without_b = BRIDGE_NATIVE_SYSTEM(api, "look_not", "never", two, 2, nullptr, 0);
    two[1].oper = BRIDGE_OPTIONAL;
    maybe_b = BRIDGE_NATIVE_SYSTEM(api, "look_maybe", "never", two, 2, nullptr, 0);
    on_a = BRIDGE_NATIVE_OBSERVER(api, "heard_a",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, one, 1, nullptr, 0);
    one[0].slot = b;
    with_b = BRIDGE_NATIVE_SYSTEM(api, "look_b", "look_b", one, 1, nullptr, 0);
    on_b = BRIDGE_NATIVE_OBSERVER(api, "heard_b",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, one, 1, nullptr, 0);
    two[1].oper = BRIDGE_AND;
    on_both = BRIDGE_NATIVE_OBSERVER(api, "heard_both",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, two, 2, nullptr, 0);
    /* an event is about the terms and not about the change: this one enters at a detach */
    two[1].oper = BRIDGE_NOT;
    on_alone = BRIDGE_NATIVE_OBSERVER(api, "heard_alone",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, two, 2, nullptr, 0);
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
