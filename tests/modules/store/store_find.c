/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/store/store_find.c -- what the plain build of
** nstore finds of its entities: each by its own id, and B by a query for it
** alone.
*/

#include "store.h"
#include "reach.h"

#if !defined(BIG) && !defined(TABLES)
uint32_t nstore_with_b;

/*
** count_named
**
** Answers how many rows of the spans of the query with A name the entity
** `id`.
*/
static uint32_t count_named(const test_api_t *api, uint64_t id)
{
    const bridge_query_t *q = test_get_query(api, nstore_with_a);
    const uint32_t *span;
    const uint64_t *who;
    uint32_t named = 0;
    uint32_t s = 0;
    uint32_t row;

    while (s < q->spans) {
        span = test_get_span(api, q, s, 1);
        who = (const uint64_t *)(api->shared_base + span[1]);
        row = 0;
        while (row < span[0]) {
            named += who[row] == id;
            row++;
        }
        s++;
    }
    return (named);
}

/*
** find
**
** Finds each entity that is left by its own id: where the index says it is,
** and what is there. Says how many are alive and found through the index
** with A, and how many times a span names one of them.
*/
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

/*
** add_values
**
** Answers `sum` and the value of every row of a column.
*/
static uint32_t add_values(const b_t *values, uint32_t count, uint32_t sum)
{
    uint32_t row = 0;

    while (row < count) {
        sum += values[row].v;
        row++;
    }
    return (sum);
}

/*
** look_b
**
** Says what a query for B alone sees: the spans, the rows, and the sum of
** the values.
*/
TEST(look_b)
{
    const bridge_query_t *q = test_get_query(api, nstore_with_b);
    const uint32_t *span;
    uint32_t rows = 0;
    uint32_t sum = 0;
    uint32_t s = 0;

    while (s < q->spans) {
        span = test_get_span(api, q, s, 1);
        rows += span[0];
        sum = add_values((const b_t *)(api->shared_base + span[2]), span[0],
            sum);
        s++;
    }
    say(api, "with B: spans, rows", q->spans, rows);
    say(api, "with B: the sum of them", sum, 0);
    return (0);
}

#endif
