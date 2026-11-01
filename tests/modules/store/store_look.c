/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "store.h"
#include "reach.h"

#if !defined(BIG) && !defined(TABLES)
uint32_t nstore_with_a;
uint32_t nstore_without_b;
uint32_t nstore_maybe_b;

static void say_first_a(const test_api_t *api, const bridge_query_t *q)
{
    const uint32_t *bases = get_column_bases(api);
    uint32_t types = test_get_header(api)->types;
    const uint32_t *span = test_get_span(api, q, 0, 1);
    uint32_t s = 0;

    while (!test_get_rows(api, span)) {
        span = test_get_span(api, q, s + 1, 1);
        s++;
    }
    say(api, "first A", ((const a_t *)(api->shared_base
        + bases[span[0] * types + nstore_a]))->v, 0);
}

static void say_with_a(const test_api_t *api)
{
    const bridge_query_t *q = test_get_query(api, nstore_with_a);
    const uint32_t *bases = get_column_bases(api);
    uint32_t types = test_get_header(api)->types;
    const uint32_t *span;
    uint32_t rows = 0;
    uint32_t empty = 0;
    uint32_t s = 0;

    while (s < q->spans) {
        span = test_get_span(api, q, s, 1);
        rows += test_get_rows(api, span);
        empty += !test_get_rows(api, span);
        if (!bases[span[0] * types + nstore_a])
            say(api, "FAIL: a matched term with no base", s, 0);
        s++;
    }
    say(api, "with A: spans, rows", q->spans, rows);
    if (empty)
        say(api, "with A: spans of no rows", empty, 0);
    if (rows)
        say_first_a(api, q);
}

static void say_without_b(const test_api_t *api)
{
    const bridge_query_t *q = test_get_query(api, nstore_without_b);
    const uint32_t *bases = get_column_bases(api);
    uint32_t types = test_get_header(api)->types;
    const uint32_t *span;
    uint32_t rows = 0;
    uint32_t s = 0;

    while (s < q->spans) {
        span = test_get_span(api, q, s, 2);
        rows += test_get_rows(api, span);
        if (bases[span[0] * types + nstore_b])
            say(api, "FAIL: a NOT term with a base", s, 0);
        s++;
    }
    say(api, "with A and not B: spans, rows", q->spans, rows);
}

static void say_maybe_b(const test_api_t *api)
{
    const bridge_query_t *q = test_get_query(api, nstore_maybe_b);
    const uint32_t *bases = get_column_bases(api);
    uint32_t types = test_get_header(api)->types;
    const uint32_t *span;
    uint32_t rows = 0;
    uint32_t have = 0;
    uint32_t s = 0;

    while (s < q->spans) {
        span = test_get_span(api, q, s, 2);
        rows += test_get_rows(api, span);
        if (bases[span[0] * types + nstore_b])
            have += test_get_rows(api, span);
        s++;
    }
    say(api, "with A and maybe B: spans, rows", q->spans, rows);
    say(api, "of them with B: rows", have, 0);
}

TEST(look)
{
    say_with_a(api);
    say_without_b(api);
    say_maybe_b(api);
    return (0);
}

#endif
