/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "direct.h"
#include "reach.h"

uint32_t ndirect_in_table;

static uint32_t add_given(const test_api_t *api, const uint32_t *span,
    uint32_t total, uint32_t *found)
{
    const bridge_header_t *h = test_get_header(api);
    const uint32_t *bases = get_column_bases(api);
    const uint32_t *blocks = get_blocks(api);
    uint32_t cells = test_get_cells(api, ndirect_d);
    const uint64_t *given = test_get_bits(api, cells);
    const d_t *cell = (const d_t *)(api->shared_base + cells);
    const uint64_t *who = (const uint64_t *)(api->shared_base
        + blocks[span[0]]);
    const a_t *column = (const a_t *)(api->shared_base
        + bases[span[0] * h->types + ndirect_a]);
    uint32_t e;
    uint32_t r = 0;

    while (r < test_get_rows(api, span)) {
        e = (uint32_t)who[r];
        if (given[e >> 6] >> (e & 63) & 1) {
            total += column[r].v + cell[e].v;
            *found += 1;
        }
        r++;
    }
    return (total);
}

TEST(look_b)
{
    const bridge_query_t *q = test_get_query(api, ndirect_in_table);
    const uint32_t *bases = get_column_bases(api);
    uint32_t types = test_get_header(api)->types;
    const uint32_t *span;
    uint32_t base;
    uint32_t rows = 0;
    uint32_t found = 0;
    uint32_t total = 0;
    uint32_t i = 0;

    while (i < q->spans) {
        span = test_get_span(api, q, i, 2);
        total = add_given(api, span, total, &found);
        base = bases[span[0] * types + ndirect_d];
        rows += test_get_rows(api, span);
        if (base)
            say(api, "FAIL: a base for a term that is direct-indexed",
                base, 0);
        i++;
    }
    say(api, "the table walked: rows, and of them given D", rows, found);
    say(api, "the table walked: the sum of A and D where both", total, 0);
    return (0);
}
