/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "stress.h"
#include "reach.h"

static uint32_t sum_spans(const test_api_t *api, const bridge_query_t *q,
    uint32_t slot, uint32_t *rows)
{
    const uint32_t *bases = get_column_bases(api);
    uint32_t types = test_get_header(api)->types;
    const uint32_t *span;
    const v_t *column;
    uint32_t sum = 0;
    uint32_t count;
    uint32_t s = 0;
    uint32_t r;

    while (s < q->spans) {
        span = test_get_span(api, q, s, 1);
        column = (const v_t *)(api->shared_base
            + bases[span[0] * types + slot]);
        count = test_get_rows(api, span);
        *rows += count;
        r = 0;
        while (r < count) {
            sum += column[r].v;
            r++;
        }
        s++;
    }
    return (sum);
}

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

int nstress_check_kind(const test_api_t *api, uint32_t kind)
{
    const bridge_query_t *q = test_get_query(api,
        nstress_seeing[kind]);
    uint32_t rows = 0;
    uint32_t sum = sum_spans(api, q, nstress_kinds[kind], &rows);

    nstress_wrong += !is_accounted(kind, rows, sum);
    if (!kind)
        nstress_check_entities(api);
    return (0);
}
