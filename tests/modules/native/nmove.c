/* One component, X, kept the way this build states: KEPT is the number of the storage layout.
 * A module is loaded as one build and reloaded as another, and X's values are to be what they
 * were. docs/bridge/data/lifetime.md, *When a component is moved*.
 *
 *     spawn        six entities with A, and X on every second one: 100, 102, 104
 *     detach_b     X taken from two of the three, so that one entity carries it
 *     detach_b_half  and from all three, so that none does
 *     flicker      a toggle: the row whose value is 102 turned off
 *     look         who carries X and what the values come to, read as this build keeps it */
#include "bridge_native.h"

BRIDGE_SCHEMA_STRUCT(A, { uint32_t v; });
BRIDGE_SCHEMA_STRUCT(X, { uint32_t v; });

constexpr uint32_t COUNT = 6;

static uint32_t a, x, reading;
static uint64_t ids[COUNT];

static void say(const bridge_api_t *api, const char *text, uint32_t n, uint32_t sum)
{
    bridge_native_log(api, BRIDGE_LOG_INFO, text, (uint32_t)strlen(text), n, sum);
}

BRIDGE_NATIVE(spawn)
{
    for (uint32_t i = 0; i < COUNT; i ++) {
        A first = { i };
        X second = { 100 + i };
        ids[i] = api->spawn(api);
        BRIDGE_NATIVE_ATTACH(api, ids[i], a, first);
#if KEPT == 5
        if (i == 4) BRIDGE_NATIVE_ATTACH(api, 0, x, second);
#else
        if (i % 2 == 0) BRIDGE_NATIVE_ATTACH(api, ids[i], x, second);
#endif
    }
    return 0;
}

BRIDGE_NATIVE(detach_b)
{
#if KEPT != 5
    api->detach(api, ids[0], x);
    api->detach(api, ids[2], x);
#endif
    return 0;
}

BRIDGE_NATIVE(detach_b_half)
{
#if KEPT != 5
    for (uint32_t i = 0; i < COUNT; i += 2) api->detach(api, ids[i], x);
#endif
    return 0;
}

BRIDGE_NATIVE(flicker)
{
#if KEPT == 3
    const bridge_query_t *q = bridge_native_get_query(api, reading);
    for (uint32_t i = 0; i < q->spans; i ++) {
        const uint32_t *span = bridge_native_get_span(api, q, i, 1);
        auto column = (const X *)(api->shared_base + span[2]);
        uint64_t *bits = bridge_native_get_bits(api, span[2]);
        for (uint32_t r = 0; r < span[0]; r ++) {
            if (column[r].v == 102) bits[r >> 6] &= ~((uint64_t)1 << (r & 63));
        }
    }
#endif
    return 0;
}

BRIDGE_NATIVE(look)
{
    uint32_t n = 0, sum = 0;
#if KEPT == 4
    uint32_t cells = bridge_native_get_cells(api, x);
    const uint64_t *given = bridge_native_get_bits(api, cells);
    for (uint32_t e = 1; e < 16; e ++) {
        if (!(given[0] >> e & 1)) continue;
        sum += ((const X *)(api->shared_base + cells))[e].v;
        n ++;
    }
    say(api, "X, direct-indexed: carried by, sum", n, sum);
#elif KEPT == 5
    const X *value = bridge_native_get_single(api, x);
    if (value) n = 1, sum = value->v;
    say(api, "X, a singleton: has a value, which is", n, sum);
#else
    const bridge_query_t *q = bridge_native_get_query(api, reading);
    for (uint32_t i = 0; i < q->spans; i ++) {
        const uint32_t *span = bridge_native_get_span(api, q, i, 1);
        auto column = (const X *)(api->shared_base + span[2]);
        for (uint32_t r = 0; r < span[0]; r ++) sum += column[r].v;
        n += span[0];
    }
    say(api, KEPT == 1 ? "X, in tables: carried by, sum"
           : KEPT == 2 ? "X, in a sparse set: carried by, sum" : "X, a toggle: carried by, sum", n, sum);
#endif
    return 0;
}

BRIDGE_NATIVE(init)
{
    int none = 0;
    bridge_term_t one[1] = {{ 0, BRIDGE_WRITE, KEPT >= 4 ? BRIDGE_OPTIONAL : BRIDGE_AND, 0 }};
    uint32_t both[2];
    a = both[0] = BRIDGE_NATIVE_COMPONENT(api, "st:A", A, BRIDGE_DELETE, BRIDGE_TABLE);
    x = both[1] = BRIDGE_NATIVE_COMPONENT(api, "st:X", X, BRIDGE_DELETE, (enum bridge_storage)KEPT);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "spawn", "spawn", &none, 0, both, 2);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "detach_b", "detach_b", &none, 0, &x, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "detach_b_half", "detach_b_half", &none, 0, &x, 1);
    one[0].slot = x;
    reading = BRIDGE_NATIVE_ADD_SYSTEM(api, "look", "look", one, 1, nullptr, 0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "flicker", "flicker", one, 1, nullptr, 0);
    return 0;
}

BRIDGE_NATIVE(reload) { return bridge_init(api); }
BRIDGE_NATIVE(shutdown) { return 0; }
