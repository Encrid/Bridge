/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/weigh/weigh_read.c -- the systems of nweigh that
** read what their own query gave them: in order, by a field tested at every
** row, and one value found a million times.
*/

#include "weigh.h"
#include "measured.h"

uint32_t nweigh_walking;
uint32_t nweigh_flagged;
volatile uint32_t nweigh_which;

/*
** walk
**
** Reads every value in order, and says their sum.
*/
TEST_MEASURED TEST(walk)
{
    const bridge_query_t *q = test_get_query(api, nweigh_walking);
    const uint32_t *span;
    const v_t *column;
    uint32_t total = 0;
    uint32_t s = 0;
    uint32_t r;

    while (s < q->spans) {
        span = test_get_span(api, q, s, 1);
        column = (const v_t *)(api->shared_base + span[2]);
        r = 0;
        while (r < span[0]) {
            total += column[r].v;
            r++;
        }
        s++;
    }
    say_read(api, total, q->spans);
    return (0);
}

/*
** add_flagged
**
** Answers `total` and the value of every row of a column whose flag is set,
** the flag being tested at every row.
*/
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

/*
** flag
**
** Reads the value of every row whose flag is set, testing the flag at every
** row, and says their sum.
*/
TEST_MEASURED TEST(flag)
{
    const bridge_query_t *q = test_get_query(api, nweigh_flagged);
    const uint32_t *span;
    uint32_t total = 0;
    uint32_t s = 0;

    while (s < q->spans) {
        span = test_get_span(api, q, s, 1);
        total = add_flagged((const f_t *)(api->shared_base + span[2]),
            span[0], total);
        s++;
    }
    say_read(api, total, q->spans);
    return (0);
}

/*
** one
**
** Reads the one value a million times, finding it again through its query
** each time, and says the sum of what it read.
*/
TEST_MEASURED TEST(one)
{
    const bridge_query_t *q;
    const uint32_t *span;
    uint32_t total = 0;
    uint32_t spans = 0;
    uint32_t i = 0;

    while (i < ASKED) {
        q = test_get_query(api, nweigh_which);
        span = test_get_span(api, q, 0, 1);
        if (q->spans && span[0]) {
            total += ((const s_t *)(api->shared_base + span[2]))->v;
            spans = q->spans;
        }
        i++;
    }
    say_read(api, total, spans);
    return (0);
}
