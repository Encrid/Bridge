/* Tags: components with no bytes, one kept in each storage layout. docs/bridge/storage.md,
 * *A tag*. A is a component of four bytes that every entity carries.
 *
 *     spawn        six entities with A. M, in tables, on every second; P, in a sparse set, on
 *                  the first two; O, a toggle, on every one; R, direct-indexed, on the odd
 *     flicker      O turned off for the first three, by their bits
 *     attach_a     W, a singleton, given
 *     detach_b     M taken from every one that has it: one table again
 *     look         what a module is handed for each
 *
 * BAD is a system that hands a tag a value, and fails for it. */
#include "bridge_native.h"

BRIDGE_SCHEMA_STRUCT(A, { uint32_t v; });

constexpr uint32_t COUNT = 6;

static uint32_t a, m, p, o, r, w, marked, in_set, toggled;
static uint64_t ids[COUNT];

static void say(const bridge_api_t *api, const char *text, uint32_t x, uint32_t y)
{
    bridge_native_log(api, BRIDGE_LOG_INFO, text, (uint32_t)strlen(text), x, y);
}

BRIDGE_NATIVE(spawn)
{
    for (uint32_t i = 0; i < COUNT; i ++) {
        A value = { i };
        ids[i] = api->spawn(api);
        bridge_native_attach(api, ids[i], a, &value, sizeof value);
        if (i % 2 == 0) api->attach(api, ids[i], m, 0, 0);
        if (i < 2) api->attach(api, ids[i], p, 0, 0);
        api->attach(api, ids[i], o, 0, 0);
        if (i % 2) api->attach(api, ids[i], r, 0, 0);
    }
#ifdef BAD
    bridge_native_attach(api, ids[0], m, &a, sizeof a);
#endif
    return 0;
}

BRIDGE_NATIVE(flicker)
{
    const bridge_query_t *q = bridge_native_get_query(api, toggled);
    for (uint32_t i = 0; i < q->spans; i ++) {
        const uint32_t *span = bridge_native_get_span(api, q, i, 2);
        auto column = (const A *)(api->shared_base + span[2]);
        uint64_t *bits = bridge_native_get_bits(api, span[3]);
        for (uint32_t row = 0; row < span[0]; row ++) {
            if (column[row].v < 3) bits[row >> 6] &= ~((uint64_t)1 << (row & 63));
        }
    }
    return 0;
}

BRIDGE_NATIVE(attach_a)
{
    api->attach(api, 0, w, 0, 0);
    return 0;
}

BRIDGE_NATIVE(detach_b)
{
    for (uint32_t i = 0; i < COUNT; i += 2) api->detach(api, ids[i], m);
    return 0;
}

BRIDGE_NATIVE(look)
{
    const bridge_header_t *h = bridge_native_get_header(api);
    const bridge_query_t *q = bridge_native_get_query(api, marked);
    const uint32_t *index = bridge_native_get_sparse_index(api, p);
    uint32_t cells = bridge_native_get_cells(api, r), rows = 0, carried = 0, on = 0, given = 0;
    const uint64_t *has = bridge_native_get_bits(api, cells);

    /* in a table: its base says it is carried, and there is nothing at it */
    for (uint32_t i = 0; i < q->spans; i ++) {
        const uint32_t *span = bridge_native_get_span(api, q, i, 2);
        rows += span[0];
        carried += span[3] == BRIDGE_CARRIED;
    }
    say(api, "M, in tables: rows that carry it, and spans whose base says carried", rows, carried);

    /* in a sparse set: walked, its base says the same; and so does each cell of its index */
    q = bridge_native_get_query(api, in_set);
    rows = carried = 0;
    for (uint32_t i = 0; i < q->spans; i ++) {
        const uint32_t *span = bridge_native_get_span(api, q, i, 1);
        rows += span[0];
        carried += span[2] == BRIDGE_CARRIED;
    }
    say(api, "P, in a sparse set: rows, and spans whose base says carried", rows, carried);
    carried = 0;
    for (uint32_t i = 0; i < COUNT; i ++) carried += index[(uint32_t)ids[i]] == BRIDGE_CARRIED;
    say(api, "P, by its index: entities whose cell says carried", carried, 0);

    /* a toggle: its bits are ahead of where its column would be, so it is handed that */
    q = bridge_native_get_query(api, toggled);
    rows = 0;
    for (uint32_t i = 0; i < q->spans; i ++) {
        const uint32_t *span = bridge_native_get_span(api, q, i, 2);
        const uint64_t *bits = bridge_native_get_bits(api, span[3]);
        for (uint32_t row = 0; row < span[0]; row ++) on += (uint32_t)(bits[row >> 6] >> (row & 63)) & 1;
        rows += span[0];
    }
    say(api, "O, a toggle: rows, and of them on", rows, on);

    /* direct-indexed: a bit an entity, and no cells behind them */
    for (uint32_t i = 0; i < COUNT; i ++) {
        uint32_t e = (uint32_t)ids[i];
        given += (uint32_t)(has[e >> 6] >> (e & 63)) & 1;
    }
    say(api, "R, direct-indexed: entities given it", given, 0);

    /* a singleton: carried, or zero */
    say(api, "W, a singleton: what its cell holds", ((const uint32_t *)(api->private_base + h->kept))[w], 0);
    return 0;
}

BRIDGE_NATIVE(init)
{
    int none = 0;
    bridge_term_t two[2] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }, { 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    uint32_t made[5];
    a = made[0] = BRIDGE_NATIVE_COMPONENT(api, "st:A", A, BRIDGE_DELETE, BRIDGE_TABLE);
    m = made[1] = BRIDGE_NATIVE_DECLARE_TAG(api, "st:M", BRIDGE_DELETE, BRIDGE_TABLE);
    p = made[2] = BRIDGE_NATIVE_DECLARE_TAG(api, "st:P", BRIDGE_DELETE, BRIDGE_SPARSE_SET);
    o = made[3] = BRIDGE_NATIVE_DECLARE_TAG(api, "st:O", BRIDGE_DELETE, BRIDGE_TOGGLE);
    r = made[4] = BRIDGE_NATIVE_DECLARE_TAG(api, "st:R", BRIDGE_DELETE, BRIDGE_DIRECT_INDEXED);
    w = BRIDGE_NATIVE_DECLARE_TAG(api, "st:W", BRIDGE_DELETE, BRIDGE_SINGLETON);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "spawn", "spawn", &none, 0, made, 5);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "attach_a", "attach_a", &none, 0, &w, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "detach_b", "detach_b", &none, 0, &m, 1);
    two[0].slot = a;
    two[1].slot = m;
    marked = BRIDGE_NATIVE_ADD_SYSTEM(api, "look", "look", two, 2, nullptr, 0);
    two[0].slot = p;
    in_set = BRIDGE_NATIVE_ADD_SYSTEM(api, "in_set", "never", two, 1, nullptr, 0);
    two[0].slot = a;
    two[1].slot = o;
    two[1].access = BRIDGE_WRITE;
    toggled = BRIDGE_NATIVE_ADD_SYSTEM(api, "flicker", "flicker", two, 2, nullptr, 0);
    return 0;
}

BRIDGE_NATIVE(in_set) { return 0; }
BRIDGE_NATIVE(shutdown) { return 0; }
BRIDGE_NATIVE(reload) { return 0; }
