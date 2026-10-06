/* A component kept in a sparse set, beside one kept in a table: docs/bridge/storage.md, *A
 * sparse set*. A is in a table and every entity carries it; S is in a sparse set and some do.
 *
 *     spawn            the entities, each with A
 *     attach_b         S for every second one, with a value of its own. BIG: for every one
 *     detach_b         and taken from each of them again
 *     detach_b_half    S taken from the first that has it
 *     value_b          a value for one that has S already
 *     attach_b_half    one more entity, with S and nothing else: it is in no table
 *     destroy_half     one that has S destroyed, and one that has not
 *     look             the set walked: a query on S alone, one span
 *     look_b           the table walked, S reached from each row through the set's index
 *     destroy          every one
 *
 * BIG is 204,800 entities and no observer, for a measurement; FAT is 10,000 that carry 4 KB.
 * BAD is a system the load is refused for: 1 selects by A and by S, 2 by not having S. */
#include "bridge_native.h"

#ifdef FAT
BRIDGE_SCHEMA_STRUCT(A, { uint32_t v; uint8_t rest[4092]; });
constexpr uint32_t COUNT = 10'000, STEP = 1;
#elifdef BIG
BRIDGE_SCHEMA_STRUCT(A, { uint32_t v; });
constexpr uint32_t COUNT = 204'800, STEP = 1;
#else
BRIDGE_SCHEMA_STRUCT(A, { uint32_t v; });
constexpr uint32_t COUNT = 6, STEP = 2;
#endif
BRIDGE_SCHEMA_STRUCT(S, { uint32_t v; });

static uint32_t a, s, in_set, in_table;
static uint64_t ids[COUNT], lone;

static void say(const bridge_api_t *api, const char *text, uint32_t x, uint32_t y)
{
    bridge_native_log(api, BRIDGE_LOG_INFO, text, (uint32_t)strlen(text), x, y);
}

BRIDGE_NATIVE(spawn)
{
    for (uint32_t i = 0; i < COUNT; i ++) {
        A value = { .v = i };
        ids[i] = api->spawn(api);
        bridge_native_attach(api, ids[i], a, &value, sizeof value);
    }
    return 0;
}

BRIDGE_NATIVE(attach_b)
{
    for (uint32_t i = 0; i < COUNT; i += STEP) {
        S value = { 100 + i };
        bridge_native_attach(api, ids[i], s, &value, sizeof value);
    }
    return 0;
}

BRIDGE_NATIVE(detach_b)
{
    for (uint32_t i = 0; i < COUNT; i += STEP) api->detach(api, ids[i], s);
    return 0;
}

BRIDGE_NATIVE(detach_b_half)
{
    api->detach(api, ids[0], s);
    return 0;
}

BRIDGE_NATIVE(value_b)
{
    S value = { 900 };
    bridge_native_attach(api, ids[2 % COUNT], s, &value, sizeof value);
    return 0;
}

BRIDGE_NATIVE(attach_b_half)
{
    S value = { 7 };
    lone = api->spawn(api);
    bridge_native_attach(api, lone, s, &value, sizeof value);
    return 0;
}

BRIDGE_NATIVE(destroy_half)
{
    api->destroy(api, ids[2 % COUNT]);
    api->destroy(api, ids[1 % COUNT]);
    return 0;
}

BRIDGE_NATIVE(destroy)
{
    for (uint32_t i = 0; i < COUNT; i ++) api->destroy(api, ids[i]);
    if (lone) api->destroy(api, lone);
    return 0;
}

/* the set, walked: one span, its rows the entities that are in it */
BRIDGE_NATIVE(look)
{
    const bridge_query_t *q = bridge_native_get_query(api, in_set);
    uint32_t rows = 0, total = 0;
    for (uint32_t i = 0; i < q->spans; i ++) {
        const uint32_t *span = bridge_native_get_span(api, q, i, 1);
        auto column = (const S *)(api->shared_base + span[2]);
        for (uint32_t r = 0; r < span[0]; r ++) total += column[r].v;
        rows += span[0];
    }
    say(api, "the set walked: spans, rows", q->spans, rows);
    say(api, "the set walked: the sum of its values", total, 0);
    return 0;
}

/* the table, walked, and S looked for at each row: its cell of the set's index is where its
 * value is, or zero */
BRIDGE_NATIVE(look_b)
{
    const bridge_query_t *q = bridge_native_get_query(api, in_table);
    const uint32_t *index = bridge_native_get_sparse_index(api, s);
    uint32_t rows = 0, found = 0, total = 0;
    for (uint32_t i = 0; i < q->spans; i ++) {
        const uint32_t *span = bridge_native_get_span(api, q, i, 2);
        auto who = (const uint64_t *)(api->shared_base + span[1]);
        auto column = (const A *)(api->shared_base + span[2]);
        for (uint32_t r = 0; r < span[0]; r ++) {
            uint32_t at = index[(uint32_t)who[r]];
            if (!at) continue;
            total += column[r].v + ((const S *)(api->shared_base + at))->v;
            found ++;
        }
        rows += span[0];
        /* it is not in the table, so the span has no base for it */
        if (span[3]) say(api, "FAIL: a base for a term kept in a sparse set", span[3], 0);
    }
    say(api, "the table walked: rows, and of them in the set", rows, found);
    say(api, "the table walked: the sum of A and S where both", total, 0);
    return 0;
}

#if !defined BIG && !defined FAT
static uint32_t on_s, on_both;

static void heard(const bridge_api_t *api, uint32_t observer, const char *text)
{
    const bridge_list_t *list = bridge_native_get_list(api, observer);
    auto trigger = (const bridge_trigger_t *)(api->private_base + list->at);
    for (uint32_t i = 0; i < list->count; i ++) say(api, text, (uint32_t)trigger[i].id, trigger[i].events);
}

BRIDGE_NATIVE(heard_s) { heard(api, on_s, "heard of S: entity, events"); return 0; }
BRIDGE_NATIVE(heard_both) { heard(api, on_both, "heard of A and S: entity, events"); return 0; }
#endif

BRIDGE_NATIVE(init)
{
    int none = 0;
    bridge_term_t one[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    bridge_term_t two[2] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }, { 0, BRIDGE_READ, BRIDGE_OPTIONAL, 0 }};
    a = BRIDGE_NATIVE_COMPONENT(api, "st:A", A, BRIDGE_DELETE, BRIDGE_TABLE);
    s = BRIDGE_NATIVE_COMPONENT(api, "st:S", S, BRIDGE_DELETE, BRIDGE_SPARSE_SET);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "spawn", "spawn", &none, 0, &a, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "attach_b", "attach_b", &none, 0, &s, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "detach_b", "detach_b", &none, 0, &s, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "detach_b_half", "detach_b_half", &none, 0, &s, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "value_b", "value_b", &none, 0, &s, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "attach_b_half", "attach_b_half", &none, 0, &s, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "destroy_half", "destroy_half", &none, 0, nullptr, 0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "destroy", "destroy", &none, 0, nullptr, 0);
    one[0].slot = s;
    in_set = BRIDGE_NATIVE_ADD_SYSTEM(api, "look", "look", one, 1, nullptr, 0);
    two[0].slot = a;
    two[1].slot = s;
    in_table = BRIDGE_NATIVE_ADD_SYSTEM(api, "look_b", "look_b", two, 2, nullptr, 0);
#if !defined BIG && !defined FAT
    on_s = BRIDGE_NATIVE_OBSERVER(api, "heard_s",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, one, 1, nullptr, 0);
    /* an observer walks nothing, so its terms may be of both */
    two[1].oper = BRIDGE_AND;
    on_both = BRIDGE_NATIVE_OBSERVER(api, "heard_both",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, two, 2, nullptr, 0);
#endif
#ifdef BAD
    two[1].oper = BAD == 1 ? BRIDGE_AND : BRIDGE_OPTIONAL;
    one[0].oper = BRIDGE_NOT;
    if (BAD == 1) BRIDGE_NATIVE_ADD_SYSTEM(api, "both", "never", two, 2, nullptr, 0);
    else BRIDGE_NATIVE_ADD_SYSTEM(api, "without", "never", one, 1, nullptr, 0);
#endif
    return 0;
}

BRIDGE_NATIVE(shutdown) { return 0; }
BRIDGE_NATIVE(reload) { return 0; }
