/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "tag.h"
#include "reach.h"

TEST(spawn)
{
    a_t value = {};

    while (value.v < COUNT) {
        ntag_ids[value.v] = api->spawn(api);
        TEST_ATTACH(api, ntag_ids[value.v], ntag_a, value);
        if (value.v % 2 == 0)
            api->attach(api, ntag_ids[value.v], ntag_m, BRIDGE_ZEROED);
        if (value.v < 2)
            api->attach(api, ntag_ids[value.v], ntag_p, BRIDGE_ZEROED);
        api->attach(api, ntag_ids[value.v], ntag_o, BRIDGE_ZEROED);
        if (value.v % 2)
            api->attach(api, ntag_ids[value.v], ntag_r, BRIDGE_ZEROED);
        value.v++;
    }
    return (0);
}

static void turn_low_off(const a_t *column, uint64_t *bits, uint32_t count)
{
    uint32_t row = 0;

    while (row < count) {
        if (column[row].v < 3)
            bits[row >> 6] &= ~((uint64_t)1 << (row & 63));
        row++;
    }
}

TEST(flicker)
{
    const bridge_query_t *q = test_get_query(api, ntag_toggled);
    const uint32_t *bases = get_column_bases(api);
    uint32_t types = test_get_header(api)->types;
    const uint32_t *span;
    uint32_t i = 0;

    while (i < q->spans) {
        span = test_get_span(api, q, i, 2);
        turn_low_off((const a_t *)(api->shared_base
            + bases[span[0] * types + ntag_a]),
            test_get_bits(api, bases[span[0] * types + ntag_o]),
            test_count_rows(api, span));
        i++;
    }
    return (0);
}

TEST(attach_a)
{
    api->attach(api, 0, ntag_w, BRIDGE_ZEROED);
    return (0);
}

TEST(detach_b)
{
    uint32_t i = 0;

    while (i < COUNT) {
        api->detach(api, ntag_ids[i], ntag_m);
        i += 2;
    }
    return (0);
}
