/* A direct-indexed component beside one kept in a table: docs/bridge/storage.md,
 * *Direct-indexed*. A is in a table and every entity carries it; D is direct-indexed and
 * some have been given one.
 *
 *     spawn            the entities, each with A
 *     attach_b         D for every second one, with a value of its own. BIG: for every one
 *     detach_b         and taken from each of them again
 *     detach_b_half    D taken from the first that has it
 *     value_b          a value for one that has D already
 *     destroy_half     one that has D destroyed, and one that has not
 *     respawn_half     two more, with A alone: they are given the places of the two that
 *                      went, and none of what was there
 *     look_b           the table walked, D reached at each row by the entity's id
 *     near, far        each entity reached by an id that is held, in the order they lie and
 *                      in no order, with the id's generation tested
 *     destroy          every one
 *
 * BIG is 204,800 entities and no observer, for a measurement. BAD is a system the load is
 * refused for: it selects by D. */
#include "bridge_native.h"

BRIDGE_SCHEMA_STRUCT(A, { uint32_t v; });
BRIDGE_SCHEMA_STRUCT(D, { uint32_t v; });

#ifdef BIG
constexpr uint32_t COUNT = 204'800, STEP = 1;
#else
constexpr uint32_t COUNT = 6, STEP = 2;
#endif

static uint32_t a, d, in_table;
static uint64_t ids[COUNT + 2], mixed[COUNT];

static void say(const bridge_api_t *api, const char *text, uint32_t x, uint32_t y)
{
    bridge_native_log(api, BRIDGE_LOG_INFO, text, (uint32_t)strlen(text), x, y);
}

BRIDGE_NATIVE(spawn)
{
    uint32_t state = 2463534242u;
    for (uint32_t i = 0; i < COUNT; i ++) {
        A value = { .v = i };
        ids[i] = api->spawn(api);
        BRIDGE_NATIVE_ATTACH(api, ids[i], a, value);
    }
    for (uint32_t i = 0; i < COUNT; i ++) {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        mixed[i] = ids[state % COUNT];
    }
    return 0;
}

BRIDGE_NATIVE(attach_b)
{
    for (uint32_t i = 0; i < COUNT; i += STEP) {
        D value = { 100 + i };
        BRIDGE_NATIVE_ATTACH(api, ids[i], d, value);
    }
    return 0;
}

BRIDGE_NATIVE(detach_b)
{
    for (uint32_t i = 0; i < COUNT; i += STEP) api->detach(api, ids[i], d);
    return 0;
}

BRIDGE_NATIVE(detach_b_half)
{
    api->detach(api, ids[0], d);
    return 0;
}

BRIDGE_NATIVE(value_b)
{
    D value = { 900 };
    BRIDGE_NATIVE_ATTACH(api, ids[2 % COUNT], d, value);
    return 0;
}

BRIDGE_NATIVE(destroy_half)
{
    api->destroy(api, ids[2 % COUNT]);
    api->destroy(api, ids[1 % COUNT]);
    return 0;
}

BRIDGE_NATIVE(respawn_half)
{
    for (uint32_t i = COUNT; i < COUNT + 2; i ++) {
        A value = { .v = i };
        ids[i] = api->spawn(api);
        BRIDGE_NATIVE_ATTACH(api, ids[i], a, value);
    }
    return 0;
}

BRIDGE_NATIVE(destroy)
{
    for (uint32_t i = 0; i < COUNT + 2; i ++) if (ids[i]) api->destroy(api, ids[i]);
    return 0;
}

/* the table walked, and D looked for at each row: the entity's own cell, if its bit says it
 * was given one */
BRIDGE_NATIVE(look_b)
{
    const bridge_query_t *q = bridge_native_get_query(api, in_table);
    uint32_t cells = bridge_native_get_cells(api, d), rows = 0, found = 0, total = 0;
    const uint64_t *given = bridge_native_get_bits(api, cells);
    auto cell = (const D *)(api->shared_base + cells);
    for (uint32_t i = 0; i < q->spans; i ++) {
        const uint32_t *span = bridge_native_get_span(api, q, i, 2);
        auto who = (const uint64_t *)(api->shared_base + span[1]);
        auto column = (const A *)(api->shared_base + span[2]);
        for (uint32_t r = 0; r < span[0]; r ++) {
            uint32_t e = (uint32_t)who[r];
            if (!(given[e >> 6] >> (e & 63) & 1)) continue;
            total += column[r].v + cell[e].v;
            found ++;
        }
        rows += span[0];
        if (span[3]) say(api, "FAIL: a base for a term that is direct-indexed", span[3], 0);
    }
    say(api, "the table walked: rows, and of them given D", rows, found);
    say(api, "the table walked: the sum of A and D where both", total, 0);
    return 0;
}

/* by ids that are held: the generation, the bit, the cell */
static int reach(const bridge_api_t *api, const uint64_t *held, uint32_t count)
{
    const bridge_header_t *h = bridge_native_get_header(api);
    auto index = (const bridge_entity_t *)(api->shared_base + h->entity_index);
    uint32_t cells = bridge_native_get_cells(api, d), alive = 0, found = 0, total = 0;
    const uint64_t *given = bridge_native_get_bits(api, cells);
    auto cell = (const D *)(api->shared_base + cells);
    for (uint32_t i = 0; i < count; i ++) {
        uint32_t e = (uint32_t)held[i];
        if (!held[i] || index[e].generation != (uint32_t)(held[i] >> 32)) continue;
        alive ++;
        if (!(given[e >> 6] >> (e & 63) & 1)) continue;
        total += cell[e].v;
        found ++;
    }
    say(api, "by id: alive, and of them given D", alive, found);
    say(api, "by id: the sum of D", total, 0);
    return 0;
}

BRIDGE_NATIVE(near) { return reach(api, ids, COUNT + 2); }
BRIDGE_NATIVE(far) { return reach(api, mixed, COUNT); }

#ifndef BIG
static uint32_t on_d, on_both;

static void heard(const bridge_api_t *api, uint32_t observer, const char *text)
{
    const bridge_list_t *list = bridge_native_get_list(api, observer);
    auto trigger = (const bridge_trigger_t *)(api->private_base + list->at);
    for (uint32_t i = 0; i < list->count; i ++) say(api, text, (uint32_t)trigger[i].id, trigger[i].events);
}

BRIDGE_NATIVE(heard_d) { heard(api, on_d, "heard of D: entity, events"); return 0; }
BRIDGE_NATIVE(heard_both) { heard(api, on_both, "heard of A and D: entity, events"); return 0; }
#endif

BRIDGE_NATIVE(init)
{
    int none = 0;
    bridge_term_t two[2] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }, { 0, BRIDGE_READ, BRIDGE_OPTIONAL, 0 }};
    a = BRIDGE_NATIVE_COMPONENT(api, "st:A", A, BRIDGE_DELETE, BRIDGE_TABLE);
    d = BRIDGE_NATIVE_COMPONENT(api, "st:D", D, BRIDGE_DELETE, BRIDGE_DIRECT_INDEXED);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "spawn", "spawn", &none, 0, &a, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "respawn_half", "respawn_half", &none, 0, &a, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "attach_b", "attach_b", &none, 0, &d, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "detach_b", "detach_b", &none, 0, &d, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "detach_b_half", "detach_b_half", &none, 0, &d, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "value_b", "value_b", &none, 0, &d, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "destroy_half", "destroy_half", &none, 0, nullptr, 0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "destroy", "destroy", &none, 0, nullptr, 0);
    two[0].slot = a;
    two[1].slot = d;
    in_table = BRIDGE_NATIVE_ADD_SYSTEM(api, "look_b", "look_b", two, 2, nullptr, 0);
    /* these two walk nothing: the term is how they say what they read */
    BRIDGE_NATIVE_ADD_SYSTEM(api, "near", "near", &two[1], 1, nullptr, 0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "far", "far", &two[1], 1, nullptr, 0);
#ifndef BIG
    two[0].slot = d;
    on_d = BRIDGE_NATIVE_OBSERVER(api, "heard_d",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, two, 1, nullptr, 0);
    two[0].slot = a;
    two[1].oper = BRIDGE_AND;
    on_both = BRIDGE_NATIVE_OBSERVER(api, "heard_both",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, two, 2, nullptr, 0);
#endif
#ifdef BAD
    two[1].oper = BRIDGE_AND;
    BRIDGE_NATIVE_ADD_SYSTEM(api, "both", "never", two, 2, nullptr, 0);
#endif
    return 0;
}

BRIDGE_NATIVE(shutdown) { return 0; }
BRIDGE_NATIVE(reload) { return 0; }
