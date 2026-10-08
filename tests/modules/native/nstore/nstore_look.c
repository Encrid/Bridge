/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/nstore/nstore_look.c -- what a system of the plain
** build of nstore sees: the spans of a query with A, of one with A and not
** B, and of one with A and maybe B.
*/

#include "nstore.h"

#if !defined(BIG) && !defined(TABLES)
uint32_t nstore_with_a;
uint32_t nstore_without_b;
uint32_t nstore_maybe_b;

/*
** say_first_a
**
** Says the first value of A: that of the first row of the first span that
** has rows.
*/
static void say_first_a(const bridge_api_t *api, const bridge_query_t *q)
{
    const uint32_t *span = bridge_native_get_span(api, q, 0, 1);
    uint32_t s = 0;

    while (!span[0]) {
        span = bridge_native_get_span(api, q, s + 1, 1);
        s++;
    }
    say(api, "first A", ((const a_t *)(api->shared_base + span[2]))->v, 0);
}

/*
** say_with_a
**
** Says the spans and the rows of the query with A, how many spans have no
** rows if any has none, and the first value. A matched term with no base is
** said as a failure.
*/
static void say_with_a(const bridge_api_t *api)
{
    const bridge_query_t *q = bridge_native_get_query(api, nstore_with_a);
    const uint32_t *span;
    uint32_t rows = 0;
    uint32_t empty = 0;
    uint32_t s = 0;

    while (s < q->spans) {
        span = bridge_native_get_span(api, q, s, 1);
        rows += span[0];
        empty += !span[0];
        if (!span[2])
            say(api, "FAIL: a matched term with no base", s, 0);
        s++;
    }
    say(api, "with A: spans, rows", q->spans, rows);
    if (empty)
        say(api, "with A: spans of no rows", empty, 0);
    if (rows)
        say_first_a(api, q);
}

/*
** say_without_b
**
** Says the spans and the rows of the query with A and not B. A term that
** is NOT has no base: one that has is said as a failure.
*/
static void say_without_b(const bridge_api_t *api)
{
    const bridge_query_t *q = bridge_native_get_query(api, nstore_without_b);
    const uint32_t *span;
    uint32_t rows = 0;
    uint32_t s = 0;

    while (s < q->spans) {
        span = bridge_native_get_span(api, q, s, 2);
        rows += span[0];
        if (span[3])
            say(api, "FAIL: a NOT term with a base", s, 0);
        s++;
    }
    say(api, "with A and not B: spans, rows", q->spans, rows);
}

/*
** say_maybe_b
**
** Says the spans and the rows of the query with A and maybe B, and how many
** of the rows have B: those of a span that has a base for it.
*/
static void say_maybe_b(const bridge_api_t *api)
{
    const bridge_query_t *q = bridge_native_get_query(api, nstore_maybe_b);
    const uint32_t *span;
    uint32_t rows = 0;
    uint32_t have = 0;
    uint32_t s = 0;

    while (s < q->spans) {
        span = bridge_native_get_span(api, q, s, 2);
        rows += span[0];
        if (span[3])
            have += span[0];
        s++;
    }
    say(api, "with A and maybe B: spans, rows", q->spans, rows);
    say(api, "of them with B: rows", have, 0);
}

/*
** look
**
** Says what three queries see: with A, with A and not B, and with A and
** maybe B.
*/
BRIDGE_NATIVE(look)
{
    say_with_a(api);
    say_without_b(api);
    say_maybe_b(api);
    return (0);
}

#endif
