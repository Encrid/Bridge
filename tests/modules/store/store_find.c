/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "store.h"
#include "reach.h"

#if !defined(BIG) && !defined(TABLES)
uint32_t nstore_with_b;

static uint32_t count_named(const test_api_t *api, uint64_t id)
{
    const bridge_query_t *q = test_get_query(api, nstore_with_a);
    const uint32_t *blocks = get_blocks(api);
    const uint32_t *span;
    const uint64_t *who;
    uint32_t named = 0;
    uint32_t s = 0;
    uint32_t row;

    while (s < q->spans) {
        span = test_get_span(api, q, s, 1);
        who = (const uint64_t *)(api->shared_base + blocks[span[0]]);
        row = 0;
        while (row < test_get_rows(api, span)) {
            named += who[row] == id;
            row++;
        }
        s++;
    }
    return (named);
}

TEST(find)
{
    const bridge_header_t *h = test_get_header(api);
    const bridge_entity_t *index = get_entity_index(api);
    const uint32_t *base = get_column_bases(api);
    const bridge_entity_t *e;
    uint32_t found = 0;
    uint32_t named = 0;
    uint32_t i = 0;

    while (i < COUNT) {
        e = &index[(uint32_t)nstore_ids[i]];
        if (e->generation == (uint32_t)(nstore_ids[i] >> 32)) {
            found += base[e->table * h->types + nstore_a] != 0;
            named += count_named(api, nstore_ids[i]);
        }
        i++;
    }
    say(api, "alive and found through the index with A, and named in a span",
        found, named);
    return (0);
}

static uint32_t add_values(const b_t *values, uint32_t count, uint32_t sum)
{
    uint32_t row = 0;

    while (row < count) {
        sum += values[row].v;
        row++;
    }
    return (sum);
}

TEST(look_b)
{
    const bridge_query_t *q = test_get_query(api, nstore_with_b);
    const uint32_t *bases = get_column_bases(api);
    uint32_t types = test_get_header(api)->types;
    const uint32_t *span;
    uint32_t rows = 0;
    uint32_t sum = 0;
    uint32_t s = 0;

    while (s < q->spans) {
        span = test_get_span(api, q, s, 1);
        rows += test_get_rows(api, span);
        sum = add_values((const b_t *)(api->shared_base
            + bases[span[0] * types + nstore_b]),
            test_get_rows(api, span), sum);
        s++;
    }
    say(api, "with B: spans, rows", q->spans, rows);
    say(api, "with B: the sum of them", sum, 0);
    return (0);
}

#endif
