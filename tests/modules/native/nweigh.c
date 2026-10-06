/* What the one storage layout there is costs at three kinds of work it was not chosen for, so
 * that a storage layout made for one of them has a figure to be weighed beside. Each is done
 * the way a module does it today. docs/plan/stage11.md, *The baseline*.
 *
 *     flag      a component an entity always has and sometimes does not use: a field says
 *               whether, and the loop tests it at every row. One row in ON has it set
 *     near      a value reached by entity id and not walked, as a reference is followed.
 *     far       The ids in the order their entities lie, and in no order
 *     one       one value for the whole deployment: a component on one entity, found through
 *               a query of one span, a million times
 *     walk      the same number of rows read in order, which is what the others are beside
 *
 * Built once for each ON: 1, 2, 16. */
#include "bridge_native.h"

BRIDGE_SCHEMA_STRUCT(V, { uint32_t v; });
BRIDGE_SCHEMA_STRUCT(F, { uint32_t on; uint32_t v; });
BRIDGE_SCHEMA_STRUCT(L, { uint64_t near; uint64_t far; });
BRIDGE_SCHEMA_STRUCT(S, { uint32_t v; });

constexpr uint32_t COUNT = 204'800, ASKED = 1'000'000;

static uint32_t value, flag, link, single, walking, flagged, linked, alone;
static uint64_t ids[COUNT];
/* which query `one` reads, behind a load the compiler cannot see through: it is to find the
 * value again each time, as a system does once a call */
static volatile uint32_t which;

static uint32_t state = 2463534242u;
static uint32_t random32()
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}

BRIDGE_NATIVE(fill)
{
    uint64_t holder = api->spawn(api);
    S s = { 1 };
    for (uint32_t i = 0; i < COUNT; i ++) ids[i] = api->spawn(api);
    for (uint32_t i = 0; i < COUNT; i ++) {
        V v = { 1 };
        F f = { random32() % ON == 0, 1 };
        L l = { ids[i], ids[random32() % COUNT] };
        BRIDGE_NATIVE_ATTACH(api, ids[i], value, v);
        BRIDGE_NATIVE_ATTACH(api, ids[i], flag, f);
        BRIDGE_NATIVE_ATTACH(api, ids[i], link, l);
    }
    BRIDGE_NATIVE_ATTACH(api, holder, single, s);
    return 0;
}

BRIDGE_NATIVE(walk)
{
    const bridge_query_t *q = bridge_native_get_query(api, walking);
    uint32_t total = 0;
    for (uint32_t s = 0; s < q->spans; s ++) {
        const uint32_t *span = bridge_native_get_span(api, q, s, 1);
        auto column = (const V *)(api->shared_base + span[2]);
        for (uint32_t r = 0; r < span[0]; r ++) total += column[r].v;
    }
    bridge_native_log(api, BRIDGE_LOG_INFO, &(bridge_line_t){ "read", 4, total, q->spans });
    return 0;
}

BRIDGE_NATIVE(flag)
{
    const bridge_query_t *q = bridge_native_get_query(api, flagged);
    uint32_t total = 0;
    for (uint32_t s = 0; s < q->spans; s ++) {
        const uint32_t *span = bridge_native_get_span(api, q, s, 1);
        auto column = (const F *)(api->shared_base + span[2]);
        for (uint32_t r = 0; r < span[0]; r ++) {
            if (column[r].on) total += column[r].v;
        }
    }
    bridge_native_log(api, BRIDGE_LOG_INFO, &(bridge_line_t){ "read", 4, total, q->spans });
    return 0;
}

/* every row's link followed to the value of the entity it names */
static int follow(const bridge_api_t *api, bool far)
{
    const bridge_header_t *h = bridge_native_get_header(api);
    const bridge_query_t *q = bridge_native_get_query(api, linked);
    auto index = (const bridge_entity_t *)(api->shared_base + h->entity_index);
    auto base = (const uint32_t *)(api->private_base + h->column_base);
    uint32_t total = 0;
    for (uint32_t s = 0; s < q->spans; s ++) {
        const uint32_t *span = bridge_native_get_span(api, q, s, 1);
        auto column = (const L *)(api->shared_base + span[2]);
        for (uint32_t r = 0; r < span[0]; r ++) {
            uint64_t id = far ? column[r].far : column[r].near;
            const bridge_entity_t *e = &index[(uint32_t)id];
            uint32_t at;
            if (e->generation != (uint32_t)(id >> 32)) continue;
            at = base[e->table * h->types + value];
            if (at) total += ((const V *)(api->shared_base + at))[e->row].v;
        }
    }
    bridge_native_log(api, BRIDGE_LOG_INFO, &(bridge_line_t){ "read", 4, total, q->spans });
    return 0;
}

BRIDGE_NATIVE(near) { return follow(api, false); }
BRIDGE_NATIVE(far) { return follow(api, true); }

BRIDGE_NATIVE(one)
{
    uint32_t total = 0, spans = 0;
    for (uint32_t i = 0; i < ASKED; i ++) {
        const bridge_query_t *q = bridge_native_get_query(api, which);
        const uint32_t *span = bridge_native_get_span(api, q, 0, 1);
        if (!q->spans || !span[0]) continue;
        total += ((const S *)(api->shared_base + span[2]))->v;
        spans = q->spans;
    }
    bridge_native_log(api, BRIDGE_LOG_INFO, &(bridge_line_t){ "read", 4, total, spans });
    return 0;
}

BRIDGE_NATIVE(init)
{
    int none = 0;
    bridge_term_t one[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    uint32_t filled[4];
    value = filled[0] = BRIDGE_NATIVE_COMPONENT(api, "st:V", V, BRIDGE_DELETE, BRIDGE_TABLE);
    flag = filled[1] = BRIDGE_NATIVE_COMPONENT(api, "st:F", F, BRIDGE_DELETE, BRIDGE_TABLE);
    link = filled[2] = BRIDGE_NATIVE_COMPONENT(api, "st:L", L, BRIDGE_DELETE, BRIDGE_TABLE);
    single = filled[3] = BRIDGE_NATIVE_COMPONENT(api, "st:S", S, BRIDGE_DELETE, BRIDGE_TABLE);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "fill", "spawn", &none, 0, filled, 4);
    one[0].slot = value;
    walking = BRIDGE_NATIVE_ADD_SYSTEM(api, "walk", "walk", one, 1, nullptr, 0);
    one[0].slot = flag;
    flagged = BRIDGE_NATIVE_ADD_SYSTEM(api, "flag", "flag", one, 1, nullptr, 0);
    one[0].slot = link;
    linked = BRIDGE_NATIVE_ADD_SYSTEM(api, "near", "near", one, 1, nullptr, 0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "far", "far", one, 1, nullptr, 0);
    one[0].slot = single;
    alone = BRIDGE_NATIVE_ADD_SYSTEM(api, "one", "one", one, 1, nullptr, 0);
    which = alone;
    return 0;
}

BRIDGE_NATIVE(shutdown) { return 0; }
BRIDGE_NATIVE(reload) { return 0; }
