/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "toggle.h"
#include "measured.h"

uint32_t ntoggle_writing;

#ifdef BIG
static uint32_t state = 2463534242u;

static uint32_t draw_number(void)
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return (state);
}

#endif

static void switch_rows([[maybe_unused]] const t_t *column, uint64_t *bits,
    uint32_t count)
{
    uint32_t r = 0;
    bool on;

    while (r < count) {
#ifdef BIG
        on = draw_number() % ON == 0;
#else
        on = column[r].v % 2 != 0;
#endif
        if (on)
            bits[r >> 6] |= (uint64_t)1 << (r & 63);
        else
            bits[r >> 6] &= ~((uint64_t)1 << (r & 63));
        r++;
    }
}

TEST_MEASURED TEST(flicker)
{
    const bridge_query_t *q = test_get_query(api, ntoggle_writing);
    const uint32_t *span;
    uint32_t i = 0;

    while (i < q->spans) {
        span = test_get_span(api, q, i, 1);
        switch_rows((const t_t *)(api->shared_base + span[2]),
            test_get_bits(api, span[2]), span[0]);
        i++;
    }
    return (0);
}

TEST(combine)
{
    const bridge_query_t *q = test_get_query(api, ntoggle_writing);
    const uint32_t *span;
    uint64_t *bits;
    uint32_t i = 0;
    uint32_t r;

    while (i < q->spans) {
        span = test_get_span(api, q, i, 1);
        bits = test_get_bits(api, span[2]);
        r = 0;
        while (r < span[0]) {
            bits[r >> 6] |= (uint64_t)1 << (r & 63);
            r++;
        }
        i++;
    }
    return (0);
}

TEST(value_a)
{
    t_t value = { 500 };

    TEST_ATTACH(api, ntoggle_ids[2 % COUNT], ntoggle_t, value);
    return (0);
}
