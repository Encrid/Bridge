/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "weigh.h"
#include "measured.h"
#include "reach.h"

uint32_t nweigh_walking;
uint32_t nweigh_flagged;
volatile uint32_t nweigh_which;

TEST_MEASURED TEST(walk)
{
    const bridge_query_t *q = test_get_query(api, nweigh_walking);
    const uint32_t *bases = get_column_bases(api);
    uint32_t types = test_get_header(api)->types;
    const uint32_t *span;
    const v_t *column;
    uint32_t total = 0;
    uint32_t count;
    uint32_t s = 0;
    uint32_t r;

    while (s < q->spans) {
        span = test_get_span(api, q, s, 1);
        column = (const v_t *)(api->shared_base
            + bases[span[0] * types + nweigh_value]);
        count = test_get_rows(api, span);
        r = 0;
        while (r < count) {
            total += column[r].v;
            r++;
        }
        s++;
    }
    say_read(api, total, q->spans);
    return (0);
}

static uint32_t add_flagged(const f_t *column, uint32_t count,
    uint32_t total)
{
    uint32_t r = 0;

    while (r < count) {
        if (column[r].on)
            total += column[r].v;
        r++;
    }
    return (total);
}

TEST_MEASURED TEST(flag)
{
    const bridge_query_t *q = test_get_query(api, nweigh_flagged);
    const uint32_t *bases = get_column_bases(api);
    uint32_t types = test_get_header(api)->types;
    const uint32_t *span;
    uint32_t total = 0;
    uint32_t s = 0;

    while (s < q->spans) {
        span = test_get_span(api, q, s, 1);
        total = add_flagged((const f_t *)(api->shared_base
            + bases[span[0] * types + nweigh_flag]),
            test_get_rows(api, span), total);
        s++;
    }
    say_read(api, total, q->spans);
    return (0);
}

TEST_MEASURED TEST(one)
{
    const uint32_t *bases = get_column_bases(api);
    uint32_t types = test_get_header(api)->types;
    const bridge_query_t *q;
    const uint32_t *span;
    uint32_t total = 0;
    uint32_t spans = 0;
    uint32_t i = 0;

    while (i < ASKED) {
        q = test_get_query(api, nweigh_which);
        span = test_get_span(api, q, 0, 1);
        if (q->spans && test_get_rows(api, span)) {
            total += ((const s_t *)(api->shared_base
                + bases[span[0] * types + nweigh_single]))->v;
            spans = q->spans;
        }
        i++;
    }
    say_read(api, total, spans);
    return (0);
}
