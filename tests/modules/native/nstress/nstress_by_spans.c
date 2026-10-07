/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/nstress/nstress_by_spans.c -- every component of
** nstress checked against its account by the spans of its query.
*/

#include "nstress.h"

/*
** sum_spans
**
** Answers the sum of the values in every span of `q`, and adds how many
** rows they have to `rows`.
*/
static uint32_t sum_spans(const bridge_api_t *api, const bridge_query_t *q,
    uint32_t *rows)
{
    const uint32_t *span;
    const v_t *column;
    uint32_t sum = 0;
    uint32_t s = 0;
    uint32_t r;

    while (s < q->spans) {
        span = bridge_native_get_span(api, q, s, 1);
        column = (const v_t *)(api->shared_base + span[2]);
        *rows += span[0];
        r = 0;
        while (r < span[0]) {
            sum += column[r].v;
            r++;
        }
        s++;
    }
    return (sum);
}

/*
** is_accounted
**
** Answers whether the account has component `kind` on `rows` entities,
** with values whose sum is `sum`.
*/
static bool is_accounted(uint32_t kind, uint32_t rows, uint32_t sum)
{
    uint32_t want_rows = 0;
    uint32_t want_sum = 0;
    uint32_t slot = 0;

    while (slot < SLOTS) {
        if (nstress_ids[slot] && (nstress_has[slot] >> kind & 1)) {
            want_rows++;
            want_sum += nstress_values[slot][kind];
        }
        slot++;
    }
    return (rows == want_rows && sum == want_sum);
}

/*
** nstress_check_kind
**
** Looks at component `kind` by its spans: as many rows as carry it, holding
** what they should. Counts a disagreement with the account, and for the
** first component checks every entity as well. Answers 0.
*/
int nstress_check_kind(const bridge_api_t *api, uint32_t kind)
{
    const bridge_query_t *q = bridge_native_get_query(api,
        nstress_seeing[kind]);
    uint32_t rows = 0;
    uint32_t sum = sum_spans(api, q, &rows);

    nstress_wrong += !is_accounted(kind, rows, sum);
    if (!kind)
        nstress_check_entities(api);
    return (0);
}
