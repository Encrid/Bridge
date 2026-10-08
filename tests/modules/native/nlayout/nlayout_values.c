/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/nlayout/nlayout_values.c -- where the values of X
** are, in each of the storage layouts a build of nlayout may keep it in.
*/

#include "nlayout.h"

/*
** collect_cells
**
** Collects from a direct-indexed X the cell of every entity below 16 whose
** bit says it was given one.
*/
static uint32_t collect_cells(const bridge_api_t *api, const x_t **out,
    uint32_t room)
{
    uint32_t cells = bridge_native_get_cells(api, x_slot);
    const uint64_t *given = bridge_native_get_bits(api, cells);
    uint32_t n = 0;
    uint32_t e = 1;

    while (e < 16 && n < room) {
        if (given[0] >> e & 1) {
            out[n] = (const x_t *)(api->shared_base + cells) + e;
            n++;
        }
        e++;
    }
    return (n);
}

static uint32_t collect_single(const bridge_api_t *api, const x_t **out,
    uint32_t room)
{
    const x_t *value = bridge_native_get_single(api, x_slot);

    if (!value || !room)
        return (0);
    out[0] = value;
    return (1);
}

/*
** collect_columns
**
** Collects the rows the query matched: X kept in tables, in a sparse set,
** or as a toggle.
*/
static uint32_t collect_columns(const bridge_api_t *api, const x_t **out,
    uint32_t room)
{
    const bridge_query_t *q = bridge_native_get_query(api, reading);
    const uint32_t *span;
    uint32_t n = 0;
    uint32_t i = 0;
    uint32_t r;

    while (i < q->spans) {
        span = bridge_native_get_span(api, q, i, 1);
        r = 0;
        while (r < span[0] && n < room) {
            out[n] = (const x_t *)(api->shared_base + span[2]) + r;
            n++;
            r++;
        }
        i++;
    }
    return (n);
}

/*
** collect_values
**
** Puts in `out` where the values of X are, however this build keeps it, and
** answers how many: `room` at the most.
*/
uint32_t collect_values(const bridge_api_t *api, const x_t **out,
    uint32_t room)
{
    if (KEPT == 4)
        return (collect_cells(api, out, room));
    if (KEPT == 5)
        return (collect_single(api, out, room));
    return (collect_columns(api, out, room));
}
