/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/nsparse/nsparse_look.c -- the systems of nsparse
** that read S: by a walk of the set, and from the rows of the table.
*/

#include "nsparse.h"
#include "native/native_measured.h"

uint32_t nsparse_in_set;
uint32_t nsparse_in_table;

/*
** add_values
**
** Answers `total` and the value of every row of a column.
*/
static uint32_t add_values(const s_t *column, uint32_t count, uint32_t total)
{
    uint32_t r = 0;

    while (r < count) {
        total += column[r].v;
        r++;
    }
    return (total);
}

/*
** look
**
** Walks the set: one span, its rows the entities that are in it. Says the
** spans, the rows, and the sum of the values.
*/
NATIVE_MEASURED BRIDGE_NATIVE(look)
{
    const bridge_query_t *q = bridge_native_get_query(api, nsparse_in_set);
    const uint32_t *span;
    uint32_t rows = 0;
    uint32_t total = 0;
    uint32_t i = 0;

    while (i < q->spans) {
        span = bridge_native_get_span(api, q, i, 1);
        total = add_values((const s_t *)(api->shared_base + span[2]), span[0],
            total);
        rows += span[0];
        i++;
    }
    say(api, "the set walked: spans, rows", q->spans, rows);
    say(api, "the set walked: the sum of its values", total, 0);
    return (0);
}

/*
** add_reached
**
** Looks for S at each row of a span of the table: the entity's cell of the
** set's index is where its value is, or zero. Answers `total` and the sum
** of A and S of each row that has both, and counts those rows in `found`.
*/
static uint32_t add_reached(const bridge_api_t *api, const uint32_t *span,
    uint32_t total, uint32_t *found)
{
    const uint32_t *index = bridge_native_get_sparse_index(api, nsparse_s);
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

/*
** look_b
**
** Walks the table, and reaches S from each row through the set's index.
** Says the rows, how many of them are in the set, and the sum of A and S
** where a row has both. S is not in the table, so a span has no base for
** it: one that has is said as a failure.
*/
NATIVE_MEASURED BRIDGE_NATIVE(look_b)
{
    const bridge_query_t *q = bridge_native_get_query(api, nsparse_in_table);
    const uint32_t *span;
    uint32_t rows = 0;
    uint32_t found = 0;
    uint32_t total = 0;
    uint32_t i = 0;

    while (i < q->spans) {
        span = bridge_native_get_span(api, q, i, 2);
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
