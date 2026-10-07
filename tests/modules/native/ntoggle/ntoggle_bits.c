/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/native/ntoggle/ntoggle_bits.c -- the systems of ntoggle
** that write T: a value for one entity, and rows turned on or off by a
** write of their bits, as a value is written, with no call and no barrier.
*/

#include "ntoggle.h"
#include "native/native_measured.h"

uint32_t ntoggle_writing;

#ifdef BIG
static uint32_t state = 2463534242u;

static uint32_t draw_number(void)
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return (state);
}

#endif

/*
** switch_rows
**
** Turns each row of a span on or off by writing its bit: on where its value
** is odd, or in a build for a measurement where one row in ON is drawn.
*/
static void switch_rows([[maybe_unused]] const t_t *column, uint64_t *bits,
    uint32_t count)
{
    uint32_t r = 0;
    bool on;

    while (r < count) {
#ifdef BIG
        on = draw_number() % ON == 0;
#else
        on = column[r].v % 2 != 0;
#endif
        if (on)
            bits[r >> 6] |= (uint64_t)1 << (r & 63);
        else
            bits[r >> 6] &= ~((uint64_t)1 << (r & 63));
        r++;
    }
}

/*
** flicker
**
** Turns off the rows whose T is even, and on the others.
*/
NATIVE_MEASURED BRIDGE_NATIVE(flicker)
{
    const bridge_query_t *q = bridge_native_get_query(api, ntoggle_writing);
    const uint32_t *span;
    uint32_t i = 0;

    while (i < q->spans) {
        span = bridge_native_get_span(api, q, i, 1);
        switch_rows((const t_t *)(api->shared_base + span[2]),
            bridge_native_get_bits(api, span[2]), span[0]);
        i++;
    }
    return (0);
}

/*
** combine
**
** Turns every row on again.
*/
BRIDGE_NATIVE(combine)
{
    const bridge_query_t *q = bridge_native_get_query(api, ntoggle_writing);
    const uint32_t *span;
    uint64_t *bits;
    uint32_t i = 0;
    uint32_t r;

    while (i < q->spans) {
        span = bridge_native_get_span(api, q, i, 1);
        bits = bridge_native_get_bits(api, span[2]);
        r = 0;
        while (r < span[0]) {
            bits[r >> 6] |= (uint64_t)1 << (r & 63);
            r++;
        }
        i++;
    }
    return (0);
}

/*
** value_a
**
** Gives a value to an entity whose T is off: it stays off.
*/
BRIDGE_NATIVE(value_a)
{
    t_t value = { 500 };

    BRIDGE_NATIVE_ATTACH(api, ntoggle_ids[2 % COUNT], ntoggle_t, value);
    return (0);
}
