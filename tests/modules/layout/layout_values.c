/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "layout.h"
#include "reach.h"

static uint32_t collect_cells(const test_api_t *api, const x_t **out,
    uint32_t room)
{
    uint32_t cells = test_get_cells(api, x_slot);
    const uint64_t *given = test_get_bits(api, cells);
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

static uint32_t collect_single(const test_api_t *api, const x_t **out,
    uint32_t room)
{
    const x_t *value = test_get_single(api, x_slot);

    if (!value || !room)
        return (0);
    out[0] = value;
    return (1);
}

static uint32_t collect_columns(const test_api_t *api, const x_t **out,
    uint32_t room)
{
    const bridge_query_t *q = test_get_query(api, reading);
    const uint32_t *bases = get_column_bases(api);
    uint32_t types = test_get_header(api)->types;
    const uint32_t *span;
    uint32_t base;
    uint32_t n = 0;
    uint32_t i = 0;
    uint32_t r;

    while (i < q->spans) {
        span = test_get_span(api, q, i, 1);
        base = bases[span[0] * types + x_slot];
        r = 0;
        while (r < test_count_rows(api, span) && n < room) {
            out[n] = (const x_t *)(api->shared_base + base) + r;
            n++;
            r++;
        }
        i++;
    }
    return (n);
}

uint32_t collect_values(const test_api_t *api, const x_t **out,
    uint32_t room)
{
    if (KEPT == 4)
        return (collect_cells(api, out, room));
    if (KEPT == 5)
        return (collect_single(api, out, room));
    return (collect_columns(api, out, room));
}
