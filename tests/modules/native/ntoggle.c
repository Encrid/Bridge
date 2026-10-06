/* A toggled component: a column of a table and a bit a row. docs/bridge/storage.md, *A
 * toggle*. A and B are kept in tables and T is a toggle; every entity carries A and T.
 *
 *     spawn            the entities, each with A and T: a toggle is on when it is given
 *     flicker          the rows whose T is even turned off, by a write of their bits
 *     combine          every row turned on again
 *     look             the rows that are on, counted a word of bits at a time, and by a test
 *                      at every row, which must agree
 *     find             the same, each entity reached by its id
 *     attach_b         B for the first half: they move to another table, as they were
 *     destroy_half     the first and the last destroyed: another row takes each place
 *     respawn_half     as many again, so that the table is moved to grow
 *     value_a          a value for one that is off: it stays off
 *
 * BIG is 204,800 entities with one in ON of them on, in no order, for a measurement. */
#include "bridge-native.h"

BRIDGE_SCHEMA_STRUCT(A, { uint32_t v; });
BRIDGE_SCHEMA_STRUCT(B, { uint32_t v; });
BRIDGE_SCHEMA_STRUCT(T, { uint32_t v; });

#ifdef BIG
constexpr uint32_t COUNT = 204'800;
#else
constexpr uint32_t COUNT = 6;
#endif

static uint32_t a, b, t, reading, writing, more;
static uint64_t ids[2 * COUNT];

static void say(const bridge_api *api, const char *text, uint32_t x, uint32_t y)
{
    bridge_native_log(api, BRIDGE_LOG_INFO, text, (uint32_t)strlen(text), x, y);
}

static void make(const bridge_api *api, uint32_t from)
{
    for (uint32_t i = from; i < from + COUNT; i ++) {
        A first = { i };
        T second = { 10 + i };
        ids[i] = api->spawn(api);
        bridge_native_attach(api, ids[i], a, &first, sizeof first);
        bridge_native_attach(api, ids[i], t, &second, sizeof second);
    }
}

BRIDGE_NATIVE(spawn) { make(api, 0); return 0; }

BRIDGE_NATIVE(respawn_half)
{
    make(api, COUNT);
    more = COUNT;
    return 0;
}

#ifdef BIG
static uint32_t state = 2463534242u;
static uint32_t random32()
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return state;
}
#endif

/* a row is turned on or off by writing its bit, as a value is written: no call, and no barrier */
BRIDGE_NATIVE(flicker)
{
    const bridge_query *q = bridge_native_query(api, writing);
    for (uint32_t i = 0; i < q->spans; i ++) {
        const uint32_t *span = bridge_native_span(api, q, i, 1);
        uint64_t *bits = bridge_native_bits(api, span[2]);
        for (uint32_t r = 0; r < span[0]; r ++) {
#ifdef BIG
            bool on = random32() % ON == 0;
#else
            bool on = ((const T *)(api->shared_base + span[2]))[r].v % 2 != 0;
#endif
            if (on) bits[r >> 6] |= (uint64_t)1 << (r & 63);
            else bits[r >> 6] &= ~((uint64_t)1 << (r & 63));
        }
    }
    return 0;
}

BRIDGE_NATIVE(combine)
{
    const bridge_query *q = bridge_native_query(api, writing);
    for (uint32_t i = 0; i < q->spans; i ++) {
        const uint32_t *span = bridge_native_span(api, q, i, 1);
        uint64_t *bits = bridge_native_bits(api, span[2]);
        for (uint32_t r = 0; r < span[0]; r ++) bits[r >> 6] |= (uint64_t)1 << (r & 63);
    }
    return 0;
}

/* the rows that are on, found a word of bits at a time */
BRIDGE_NATIVE(look)
{
    const bridge_query *q = bridge_native_query(api, reading);
    uint32_t rows = 0, on = 0, total = 0;
    for (uint32_t i = 0; i < q->spans; i ++) {
        const uint32_t *span = bridge_native_span(api, q, i, 1);
        auto column = (const T *)(api->shared_base + span[2]);
        const uint64_t *bits = bridge_native_bits(api, span[2]);
        const uint32_t count = span[0], whole = count / 64;
        for (uint32_t w = 0; w <= whole; w ++) {
            uint64_t word = bits[w];
            /* the last word has bits past the last row, which are nobody's */
            if (w == whole) word = count % 64 ? word & (((uint64_t)1 << (count % 64)) - 1) : 0;
            /* sixty-four rows that are all on are read as a column is */
            if (word == UINT64_MAX) {
                for (uint32_t r = w * 64; r < w * 64 + 64; r ++) total += column[r].v;
                on += 64;
                continue;
            }
            for (; word; word &= word - 1) {
                total += column[w * 64 + (uint32_t)__builtin_ctzll(word)].v;
                on ++;
            }
        }
        rows += count;
    }
    say(api, "by words: rows, and of them on", rows, on);
    say(api, "by words: the sum of those on, and spans", total, q->spans);
    return 0;
}

/* and tested at every row */
BRIDGE_NATIVE(look_b)
{
    const bridge_query *q = bridge_native_query(api, reading);
    uint32_t on = 0, total = 0;
    for (uint32_t i = 0; i < q->spans; i ++) {
        const uint32_t *span = bridge_native_span(api, q, i, 1);
        const T *restrict column = (const T *)(api->shared_base + span[2]);
        const uint64_t *restrict bits = bridge_native_bits(api, span[2]);
        const uint32_t count = span[0];
        for (uint32_t r = 0; r < count; r ++) {
            uint32_t is = (uint32_t)(bits[r >> 6] >> (r & 63)) & 1;
            total += column[r].v & (0u - is);
            on += is;
        }
    }
    say(api, "by rows: on, and their sum", on, total);
    return 0;
}

/* each entity reached by its id: the column through column_base, and the bits ahead of it */
BRIDGE_NATIVE(find)
{
    const bridge_header *h = bridge_native_header(api);
    auto index = (const bridge_entity *)(api->shared_base + h->entity_index);
    auto base = (const uint32_t *)(api->private_base + h->column_base);
    uint32_t alive = 0, on = 0, total = 0;
    for (uint32_t i = 0; i < COUNT + more; i ++) {
        const bridge_entity *e = &index[(uint32_t)ids[i]];
        uint32_t column;
        if (e->generation != (uint32_t)(ids[i] >> 32)) continue;
        alive ++;
        column = base[e->table * h->types + t];
        if (!column || !(bridge_native_bits(api, column)[e->row >> 6] >> (e->row & 63) & 1)) continue;
        total += ((const T *)(api->shared_base + column))[e->row].v;
        on ++;
    }
    say(api, "by id: alive, and of them on", alive, on);
    say(api, "by id: the sum of those on", total, 0);
    return 0;
}

BRIDGE_NATIVE(attach_b)
{
    for (uint32_t i = 0; i < COUNT / 2; i ++) {
        B value = { i };
        bridge_native_attach(api, ids[i], b, &value, sizeof value);
    }
    return 0;
}

BRIDGE_NATIVE(destroy_half)
{
    api->destroy(api, ids[0]);
    api->destroy(api, ids[COUNT - 1]);
    return 0;
}

BRIDGE_NATIVE(value_a)
{
    T value = { 500 };
    bridge_native_attach(api, ids[2 % COUNT], t, &value, sizeof value);
    return 0;
}

BRIDGE_NATIVE(init)
{
    int none = 0;
    bridge_term one[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    uint32_t made[2];
    a = made[0] = BRIDGE_NATIVE_COMPONENT(api, "st:A", A, BRIDGE_DELETE, BRIDGE_TABLE);
    b = BRIDGE_NATIVE_COMPONENT(api, "st:B", B, BRIDGE_DELETE, BRIDGE_TABLE);
    t = made[1] = BRIDGE_NATIVE_COMPONENT(api, "st:T", T, BRIDGE_DELETE, BRIDGE_TOGGLE);
    BRIDGE_NATIVE_SYSTEM(api, "spawn", "spawn", &none, 0, made, 2);
    BRIDGE_NATIVE_SYSTEM(api, "respawn_half", "respawn_half", &none, 0, made, 2);
    BRIDGE_NATIVE_SYSTEM(api, "attach_b", "attach_b", &none, 0, &b, 1);
    BRIDGE_NATIVE_SYSTEM(api, "destroy_half", "destroy_half", &none, 0, nullptr, 0);
    BRIDGE_NATIVE_SYSTEM(api, "value_a", "value_a", &none, 0, &t, 1);
    BRIDGE_NATIVE_SYSTEM(api, "find", "find", &none, 0, nullptr, 0);
    one[0].slot = t;
    reading = BRIDGE_NATIVE_SYSTEM(api, "look", "look", one, 1, nullptr, 0);
    BRIDGE_NATIVE_SYSTEM(api, "look_b", "look_b", one, 1, nullptr, 0);
    one[0].access = BRIDGE_WRITE;
    writing = BRIDGE_NATIVE_SYSTEM(api, "flicker", "flicker", one, 1, nullptr, 0);
    BRIDGE_NATIVE_SYSTEM(api, "combine", "combine", one, 1, nullptr, 0);
    return 0;
}

BRIDGE_NATIVE(shutdown) { return 0; }
BRIDGE_NATIVE(reload) { return 0; }
