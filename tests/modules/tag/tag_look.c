/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "tag.h"
#include "reach.h"

static void look_tables(const test_api_t *api)
{
    const bridge_query_t *q = test_get_query(api, ntag_marked);
    const uint32_t *bases = get_column_bases(api);
    uint32_t types = test_get_header(api)->types;
    const uint32_t *span;
    uint32_t rows = 0;
    uint32_t carried = 0;
    uint32_t i = 0;

    while (i < q->spans) {
        span = test_get_span(api, q, i, 2);
        rows += test_get_rows(api, span);
        carried += bases[span[0] * types + ntag_m] == BRIDGE_CARRIED;
        i++;
    }
    say(api,
        "M, in tables: rows that carry it, and spans whose base says carried",
        rows, carried);
}

static void look_toggle(const test_api_t *api)
{
    const bridge_query_t *q = test_get_query(api, ntag_toggled);
    const uint32_t *bases = get_column_bases(api);
    uint32_t types = test_get_header(api)->types;
    const uint32_t *span;
    const uint64_t *bits;
    uint32_t rows = 0;
    uint32_t on = 0;
    uint32_t i = 0;
    uint32_t row;

    while (i < q->spans) {
        span = test_get_span(api, q, i, 2);
        bits = test_get_bits(api, bases[span[0] * types + ntag_o]);
        row = 0;
        while (row < test_get_rows(api, span)) {
            on += (uint32_t)(bits[row >> 6] >> (row & 63)) & 1;
            row++;
        }
        rows += test_get_rows(api, span);
        i++;
    }
    say(api, "O, a toggle: rows, and of them on", rows, on);
}

static void look_direct(const test_api_t *api)
{
    uint32_t cells = test_get_cells(api, ntag_r);
    const uint64_t *has = test_get_bits(api, cells);
    uint32_t given = 0;
    uint32_t i = 0;
    uint32_t e;

    while (i < COUNT) {
        e = (uint32_t)ntag_ids[i];
        given += (uint32_t)(has[e >> 6] >> (e & 63)) & 1;
        i++;
    }
    say(api, "R, direct-indexed: entities given it", given, 0);
}

TEST(look)
{
    const bridge_header_t *h = test_get_header(api);
    const uint32_t *kept = (const uint32_t *)(api->private_base + h->kept);

    look_tables(api);
    ntag_look_set(api);
    look_toggle(api);
    look_direct(api);
    say(api, "W, a singleton: what its cell holds", kept[ntag_w], 0);
    return (0);
}
