/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "move.h"

static const char *const TEXTS[5] = {
    "X, in tables: carried by, sum",
    "X, in a sparse set: carried by, sum",
    "X, a toggle: carried by, sum",
    "X, direct-indexed: carried by, sum",
    "X, a singleton: has a value, which is"
};

static void add_cells(const test_api_t *api, bridge_line_t *line)
{
    uint32_t cells = test_get_cells(api, x_slot);
    const uint64_t *given = test_get_bits(api, cells);
    const x_t *cell = (const x_t *)(api->shared_base + cells);
    uint32_t e = 1;

    while (e < 16) {
        if (given[0] >> e & 1) {
            line->a1 += cell[e].v;
            line->a0++;
        }
        e++;
    }
}

static void add_single(const test_api_t *api, bridge_line_t *line)
{
    const x_t *value = test_get_single(api, x_slot);

    if (!value)
        return;
    line->a0 = 1;
    line->a1 = value->v;
}

static void add_columns(const test_api_t *api, bridge_line_t *line)
{
    const bridge_query_t *q = test_get_query(api, reading);
    const uint32_t *span;
    const x_t *column;
    uint32_t i = 0;
    uint32_t r;

    while (i < q->spans) {
        span = test_get_span(api, q, i, 1);
        column = (const x_t *)(api->shared_base + span[2]);
        r = 0;
        while (r < span[0]) {
            line->a1 += column[r].v;
            r++;
        }
        line->a0 += span[0];
        i++;
    }
}

TEST(look)
{
    const char *text = TEXTS[KEPT - 1];
    bridge_line_t line = { text, (uint32_t)strlen(text), 0, 0 };

    if (KEPT == 4)
        add_cells(api, &line);
    else if (KEPT == 5)
        add_single(api, &line);
    else
        add_columns(api, &line);
    test_log(api, BRIDGE_LOG_INFO, &line);
    return (0);
}
