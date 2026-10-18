/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "ndirect.h"

uint32_t ndirect_in_table;

static uint32_t add_given(const bridge_api_t *api, const uint32_t *span,
    uint32_t total, uint32_t *found)
{
    uint32_t cells = bridge_native_get_cells(api, ndirect_d);
    const uint64_t *given = bridge_native_get_bits(api, cells);
    const d_t *cell = (const d_t *)(api->shared_base + cells);
    const uint64_t *who = (const uint64_t *)(api->shared_base + span[1]);
    const a_t *column = (const a_t *)(api->shared_base + span[2]);
    uint32_t e;
    uint32_t r = 0;

    while (r < span[0]) {
        e = (uint32_t)who[r];
        if (given[e >> 6] >> (e & 63) & 1) {
            total += column[r].v + cell[e].v;
            *found += 1;
        }
        r++;
    }
    return (total);
}

BRIDGE_NATIVE(look_b)
{
    const bridge_query_t *q = bridge_native_get_query(api, ndirect_in_table);
    const uint32_t *span;
    uint32_t rows = 0;
    uint32_t found = 0;
    uint32_t total = 0;
    uint32_t i = 0;

    while (i < q->spans) {
        span = bridge_native_get_span(api, q, i, 2);
        total = add_given(api, span, total, &found);
        rows += span[0];
        if (span[3])
            say(api, "FAIL: a base for a term that is direct-indexed",
                span[3], 0);
        i++;
    }
    say(api, "the table walked: rows, and of them given D", rows, found);
    say(api, "the table walked: the sum of A and D where both", total, 0);
    return (0);
}
