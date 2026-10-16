/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "weigh.h"
#include "measured.h"
#include "reach.h"

uint32_t nweigh_value;
uint32_t nweigh_linked;

static uint32_t add_followed(const test_api_t *api, const uint32_t *span,
    bool far, uint32_t total)
{
    const bridge_header_t *h = test_get_header(api);
    const bridge_entity_t *index = get_entity_index(api);
    const uint32_t *base = get_column_bases(api);
    const l_t *column = (const l_t *)(api->shared_base + span[2]);
    const bridge_entity_t *e;
    uint64_t id;
    uint32_t at;
    uint32_t r = 0;

    while (r < span[0]) {
        id = far ? column[r].far : column[r].near;
        e = &index[(uint32_t)id];
        at = 0;
        if (e->generation == (uint32_t)(id >> 32))
            at = base[e->table * h->types + nweigh_value];
        if (at)
            total += ((const v_t *)(api->shared_base + at))[e->row].v;
        r++;
    }
    return (total);
}

TEST_MEASURED static int follow(const test_api_t *api, bool far)
{
    const bridge_query_t *q = test_get_query(api, nweigh_linked);
    uint32_t total = 0;
    uint32_t s = 0;

    while (s < q->spans) {
        total = add_followed(api, test_get_span(api, q, s, 1), far,
            total);
        s++;
    }
    say_read(api, total, q->spans);
    return (0);
}

TEST(near)
{
    return (follow(api, false));
}

TEST(far)
{
    return (follow(api, true));
}
