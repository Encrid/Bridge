/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "nsparse.h"
#include "native_measured.h"
#include "native_reach.h"

uint32_t nsparse_in_set;
uint32_t nsparse_in_table;

static uint32_t add_values(const s_t *column, uint32_t count, uint32_t total)
{
    uint32_t r = 0;

    while (r < count) {
        total += column[r].v;
        r++;
    }
    return (total);
}

NATIVE_MEASURED BRIDGE_NATIVE(look)
{
    const bridge_query_t *q = bridge_native_get_query(api, nsparse_in_set);
    const uint32_t *bases = get_column_bases(api);
    uint32_t types = bridge_native_get_header(api)->types;
    const uint32_t *span;
    uint32_t rows = 0;
    uint32_t total = 0;
    uint32_t i = 0;

    while (i < q->spans) {
        span = bridge_native_get_span(api, q, i, 1);
        total = add_values((const s_t *)(api->shared_base
            + bases[span[0] * types + nsparse_s]), span[1], total);
        rows += span[1];
        i++;
    }
    say(api, "the set walked: spans, rows", q->spans, rows);
    say(api, "the set walked: the sum of its values", total, 0);
    return (0);
}

static uint32_t add_reached(const bridge_api_t *api, const uint32_t *span,
    uint32_t total, uint32_t *found)
{
    const bridge_header_t *h = bridge_native_get_header(api);
    const uint32_t *bases = get_column_bases(api);
    const uint32_t *blocks = get_blocks(api);
    const uint32_t *index = bridge_native_get_sparse_index(api, nsparse_s);
    const uint64_t *who = (const uint64_t *)(api->shared_base
        + blocks[span[0]]);
    const a_t *column = (const a_t *)(api->shared_base
        + bases[span[0] * h->types + nsparse_a]);
    uint32_t at;
    uint32_t r = 0;

    while (r < span[1]) {
        at = index[(uint32_t)who[r]];
        if (at) {
            total += column[r].v + ((const s_t *)(api->shared_base + at))->v;
            *found += 1;
        }
        r++;
    }
    return (total);
}

NATIVE_MEASURED BRIDGE_NATIVE(look_b)
{
    const bridge_query_t *q = bridge_native_get_query(api, nsparse_in_table);
    const uint32_t *bases = get_column_bases(api);
    uint32_t types = bridge_native_get_header(api)->types;
    const uint32_t *span;
    uint32_t base;
    uint32_t rows = 0;
    uint32_t found = 0;
    uint32_t total = 0;
    uint32_t i = 0;

    while (i < q->spans) {
        span = bridge_native_get_span(api, q, i, 2);
        total = add_reached(api, span, total, &found);
        base = bases[span[0] * types + nsparse_s];
        rows += span[1];
        if (base)
            say(api, "FAIL: a base for a term kept in a sparse set", base,
                0);
        i++;
    }
    say(api, "the table walked: rows, and of them in the set", rows, found);
    say(api, "the table walked: the sum of A and S where both", total, 0);
    return (0);
}
