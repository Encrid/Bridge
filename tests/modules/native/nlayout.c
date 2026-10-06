/* One component, X, whose struct this build states: V says which. A module is loaded as V 0
 * and reloaded as another, and what its values become is docs/bridge/data/lifetime.md, *When a
 * component layout changed*. KEPT is the storage layout X is kept in, the same in both.
 *
 *     V  0   a, b, arr[4], at { x, y }, mode { Off, On, Auto }
 *        1   a field added
 *        2   the fields in another order
 *        3   arr made longer
 *        4   b removed
 *        5   arr made shorter
 *        6   b renamed
 *        7   b of another type
 *        8   the struct `at` names changed: its fields in another order, and one added
 *        9   the enum's constants in another order, so that two have another number
 *       10   a constant added to the enum
 *       11   a field removed from the struct `at` names
 *       12   a constant removed from the enum
 *       13   the struct `at` named declared as a tag, and X without that field
 *
 *     spawn    three entities with X: a 1000.., b 20.., arr 1 2 3 4 and up, at 5 6 and up, On
 *     look     every field this build has, summed over the values there are */
#include "bridge_native.h"

#if V == 9
BRIDGE_SCHEMA_ENUM(Mode, int8_t, { Off, Auto, On });
#elif V == 10
BRIDGE_SCHEMA_ENUM(Mode, int8_t, { Off, On, Auto, Max });
#elif V == 12
BRIDGE_SCHEMA_ENUM(Mode, int8_t, { Off, On });
#else
BRIDGE_SCHEMA_ENUM(Mode, int8_t, { Off, On, Auto });
#endif

#if V == 13
typedef struct { uint16_t x, y; } Vec;
#elif V == 8
BRIDGE_SCHEMA_STRUCT(Vec, { uint16_t y; uint16_t x; uint16_t z; });
#elif V == 11
BRIDGE_SCHEMA_STRUCT(Vec, { uint16_t x; });
#else
BRIDGE_SCHEMA_STRUCT(Vec, { uint16_t x; uint16_t y; });
#endif

#if V == 1
BRIDGE_SCHEMA_STRUCT(X, { uint32_t a; uint16_t b; uint8_t arr[4]; Vec at; Mode mode; uint32_t added; });
#elif V == 2
BRIDGE_SCHEMA_STRUCT(X, { Mode mode; Vec at; uint8_t arr[4]; uint16_t b; uint32_t a; });
#elif V == 3
BRIDGE_SCHEMA_STRUCT(X, { uint32_t a; uint16_t b; uint8_t arr[6]; Vec at; Mode mode; });
#elif V == 4
BRIDGE_SCHEMA_STRUCT(X, { uint32_t a; uint8_t arr[4]; Vec at; Mode mode; });
#elif V == 5
BRIDGE_SCHEMA_STRUCT(X, { uint32_t a; uint16_t b; uint8_t arr[2]; Vec at; Mode mode; });
#elif V == 6
BRIDGE_SCHEMA_STRUCT(X, { uint32_t a; uint16_t bee; uint8_t arr[4]; Vec at; Mode mode; });
#elif V == 7
BRIDGE_SCHEMA_STRUCT(X, { uint32_t a; uint32_t b; uint8_t arr[4]; Vec at; Mode mode; });
#elif V == 13
BRIDGE_SCHEMA_STRUCT(X, { uint32_t a; uint16_t b; uint8_t arr[4]; Mode mode; Vec at; });
#else
BRIDGE_SCHEMA_STRUCT(X, { uint32_t a; uint16_t b; uint8_t arr[4]; Vec at; Mode mode; });
#endif

static uint32_t x, reading;

static void say(const bridge_api_t *api, const char *text, uint32_t first, uint32_t second)
{
    bridge_native_log(api, BRIDGE_LOG_INFO, text, (uint32_t)strlen(text), first, second);
}

#if V == 0
BRIDGE_NATIVE(spawn)
{
    for (uint32_t i = 0; i < (KEPT == 5 ? 1 : 3); i ++) {
        X value = { .a = 1000 + i, .b = (uint16_t)(20 + i), .at = { (uint16_t)(5 + i), (uint16_t)(6 + i) },
                    .mode = On };
        for (uint32_t k = 0; k < 4; k ++) value.arr[k] = (uint8_t)(1 + k + i);
        bridge_native_attach(api, KEPT == 5 ? 0 : api->spawn(api), x, &value, sizeof value);
    }
    return 0;
}
#else
BRIDGE_NATIVE(spawn) { return 0; }
#endif

/* the values of X there are, however this build keeps it: how many, up to `room` */
static uint32_t values(const bridge_api_t *api, const X **out, uint32_t room)
{
    uint32_t n = 0;
#if KEPT == 4
    uint32_t cells = bridge_native_cells(api, x);
    const uint64_t *given = bridge_native_bits(api, cells);
    for (uint32_t e = 1; e < 16 && n < room; e ++) {
        if (given[0] >> e & 1) out[n ++] = (const X *)(api->shared_base + cells) + e;
    }
#elif KEPT == 5
    const X *value = bridge_native_single(api, x);
    if (value && room) out[n ++] = value;
#else
    const bridge_query_t *q = bridge_native_query(api, reading);
    for (uint32_t i = 0; i < q->spans; i ++) {
        const uint32_t *span = bridge_native_span(api, q, i, 1);
        for (uint32_t r = 0; r < span[0] && n < room; r ++) {
            out[n ++] = (const X *)(api->shared_base + span[2]) + r;
        }
    }
#endif
    return n;
}

BRIDGE_NATIVE(look)
{
    const X *value[8];
    uint32_t n = values(api, value, 8), a = 0, b = 0, arr = 0, at_x = 0, at_y = 0, mode = 0, more = 0;
    for (uint32_t i = 0; i < n; i ++) {
        a += value[i]->a;
#if V == 6
        b += value[i]->bee;
#elif V != 4
        b += value[i]->b;
#endif
        for (uint32_t k = 0; k < sizeof value[i]->arr; k ++) arr += value[i]->arr[k];
        at_x += value[i]->at.x;
#if V != 11
        at_y += value[i]->at.y;
#endif
#if V == 8
        more += value[i]->at.z;
#elif V == 1
        more += value[i]->added;
#endif
        mode += (uint32_t)value[i]->mode;
    }
    say(api, "values, and the sum of a", n, a);
    say(api, "the sums of b, and of arr", b, arr);
    say(api, "the sums of at.x, and of at.y", at_x, at_y);
    say(api, "the sums of mode, and of what was added", mode, more);
    return 0;
}

BRIDGE_NATIVE(init)
{
    int none = 0;
    bridge_term_t one[1] = {{ 0, BRIDGE_READ, KEPT >= 4 ? BRIDGE_OPTIONAL : BRIDGE_AND, 0 }};
    BRIDGE_NATIVE_DECLARE(api, BRIDGE_ENUM, "st:Mode", Mode, BRIDGE_STRIP, BRIDGE_TABLE);
#if V == 13
    BRIDGE_NATIVE_DECLARE_TAG(api, "st:Vec", BRIDGE_STRIP, BRIDGE_TABLE);
#else
    BRIDGE_NATIVE_DECLARE(api, BRIDGE_STRUCT, "st:Vec", Vec, BRIDGE_STRIP, BRIDGE_TABLE);
#endif
    x = BRIDGE_NATIVE_COMPONENT(api, "st:X", X, BRIDGE_DELETE, (enum bridge_storage)KEPT);
    BRIDGE_NATIVE_SYSTEM(api, "spawn", "spawn", &none, 0, &x, 1);
    one[0].slot = x;
    reading = BRIDGE_NATIVE_SYSTEM(api, "look", "look", one, 1, nullptr, 0);
    return 0;
}

BRIDGE_NATIVE(reload) { return bridge_init(api); }
BRIDGE_NATIVE(shutdown) { return 0; }
