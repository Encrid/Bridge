/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "move.h"
#include "reach.h"

TEST(spawn)
{
    a_t first = {};
    x_t second = {};
    uint32_t i = 0;

    while (i < COUNT) {
        first.v = i;
        second.v = 100 + i;
        ids[i] = api->spawn(api);
        TEST_ATTACH(api, ids[i], a_slot, first);
        if (KEPT == 5 && i == 4)
            TEST_ATTACH(api, 0, x_slot, second);
        if (KEPT != 5 && i % 2 == 0)
            TEST_ATTACH(api, ids[i], x_slot, second);
        i++;
    }
    return (0);
}

TEST(detach_b)
{
    if (KEPT == 5)
        return (0);
    api->detach(api, ids[0], x_slot);
    api->detach(api, ids[2], x_slot);
    return (0);
}

TEST(detach_b_half)
{
    uint32_t i = 0;

    if (KEPT == 5)
        return (0);
    while (i < COUNT) {
        api->detach(api, ids[i], x_slot);
        i += 2;
    }
    return (0);
}

static void turn_off(const test_api_t *api, const uint32_t *span)
{
    const uint32_t *bases = get_column_bases(api);
    uint32_t types = test_get_header(api)->types;
    uint32_t base = bases[span[0] * types + x_slot];
    const x_t *column = (const x_t *)(api->shared_base + base);
    uint64_t *bits = test_get_bits(api, base);
    uint32_t r = 0;

    while (r < test_get_rows(api, span)) {
        if (column[r].v == 102)
            bits[r >> 6] &= ~((uint64_t)1 << (r & 63));
        r++;
    }
}

TEST(flicker)
{
    const bridge_query_t *q;
    uint32_t i = 0;

    if (KEPT != 3)
        return (0);
    q = test_get_query(api, reading);
    while (i < q->spans) {
        turn_off(api, test_get_span(api, q, i, 1));
        i++;
    }
    return (0);
}
