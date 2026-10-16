/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "sparse.h"
#include "measured.h"

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

TEST_MEASURED TEST(look)
{
    const bridge_query_t *q = test_get_query(api, nsparse_in_set);
    const uint32_t *span;
    uint32_t rows = 0;
    uint32_t total = 0;
    uint32_t i = 0;

    while (i < q->spans) {
        span = test_get_span(api, q, i, 1);
        total = add_values((const s_t *)(api->shared_base + span[2]), span[0],
            total);
        rows += span[0];
        i++;
    }
    say(api, "the set walked: spans, rows", q->spans, rows);
    say(api, "the set walked: the sum of its values", total, 0);
    return (0);
}

static uint32_t add_reached(const test_api_t *api, const uint32_t *span,
    uint32_t total, uint32_t *found)
{
    const uint32_t *index = test_get_sparse_index(api, nsparse_s);
    const uint64_t *who = (const uint64_t *)(api->shared_base + span[1]);
    const a_t *column = (const a_t *)(api->shared_base + span[2]);
    uint32_t at;
    uint32_t r = 0;

    while (r < span[0]) {
        at = index[(uint32_t)who[r]];
        if (at) {
            total += column[r].v + ((const s_t *)(api->shared_base + at))->v;
            *found += 1;
        }
        r++;
    }
    return (total);
}

TEST_MEASURED TEST(look_b)
{
    const bridge_query_t *q = test_get_query(api, nsparse_in_table);
    const uint32_t *span;
    uint32_t rows = 0;
    uint32_t found = 0;
    uint32_t total = 0;
    uint32_t i = 0;

    while (i < q->spans) {
        span = test_get_span(api, q, i, 2);
        total = add_reached(api, span, total, &found);
        rows += span[0];
        if (span[3])
            say(api, "FAIL: a base for a term kept in a sparse set", span[3],
                0);
        i++;
    }
    say(api, "the table walked: rows, and of them in the set", rows, found);
    say(api, "the table walked: the sum of A and S where both", total, 0);
    return (0);
}
