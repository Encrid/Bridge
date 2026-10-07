/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/native/ntag/ntag_look.c -- what a module is handed for a
** tag in each storage layout, said in the log.
*/

#include "ntag.h"

/*
** look_tables
**
** Says of M, kept in tables, how many rows carry it and how many spans have
** a base that says carried: there is nothing at such a base.
*/
static void look_tables(const bridge_api_t *api)
{
    const bridge_query_t *q = bridge_native_get_query(api, ntag_marked);
    const uint32_t *span;
    uint32_t rows = 0;
    uint32_t carried = 0;
    uint32_t i = 0;

    while (i < q->spans) {
        span = bridge_native_get_span(api, q, i, 2);
        rows += span[0];
        carried += span[3] == BRIDGE_CARRIED;
        i++;
    }
    say(api,
        "M, in tables: rows that carry it, and spans whose base says carried",
        rows, carried);
}

/*
** look_toggle
**
** Says of O, a toggle, how many rows have it and how many of them are on.
** Its bits are ahead of where its column would be, so the base it is handed
** is that place.
*/
static void look_toggle(const bridge_api_t *api)
{
    const bridge_query_t *q = bridge_native_get_query(api, ntag_toggled);
    const uint32_t *span;
    const uint64_t *bits;
    uint32_t rows = 0;
    uint32_t on = 0;
    uint32_t i = 0;
    uint32_t row;

    while (i < q->spans) {
        span = bridge_native_get_span(api, q, i, 2);
        bits = bridge_native_get_bits(api, span[3]);
        row = 0;
        while (row < span[0]) {
            on += (uint32_t)(bits[row >> 6] >> (row & 63)) & 1;
            row++;
        }
        rows += span[0];
        i++;
    }
    say(api, "O, a toggle: rows, and of them on", rows, on);
}

/*
** look_direct
**
** Says of R, direct-indexed, how many entities were given it: a bit an
** entity, and no cells behind the bits.
*/
static void look_direct(const bridge_api_t *api)
{
    uint32_t cells = bridge_native_get_cells(api, ntag_r);
    const uint64_t *has = bridge_native_get_bits(api, cells);
    uint32_t given = 0;
    uint32_t i = 0;
    uint32_t e;

    while (i < COUNT) {
        e = (uint32_t)ntag_ids[i];
        given += (uint32_t)(has[e >> 6] >> (e & 63)) & 1;
        i++;
    }
    say(api, "R, direct-indexed: entities given it", given, 0);
}

/*
** look
**
** Says what a module is handed for a tag in each storage layout. The cell
** of W, a singleton, holds that it is carried, or zero.
*/
BRIDGE_NATIVE(look)
{
    const bridge_header_t *h = bridge_native_get_header(api);
    const uint32_t *kept = (const uint32_t *)(api->private_base + h->kept);

    look_tables(api);
    ntag_look_set(api);
    look_toggle(api);
    look_direct(api);
    say(api, "W, a singleton: what its cell holds", kept[ntag_w], 0);
    return (0);
}
