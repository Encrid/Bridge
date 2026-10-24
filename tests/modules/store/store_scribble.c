/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "store.h"
#include "reach.h"

#if !defined(BIG) && !defined(TABLES)
static void overwrite_index(const test_api_t *api)
{
    bridge_entity_t *index = get_entity_index(api);
    bridge_entity_t first = index[(uint32_t)nstore_ids[0]];
    uint32_t i = 0;

    while (i < COUNT) {
        index[(uint32_t)nstore_ids[i]] = first;
        i++;
    }
}

static void overwrite_names(const test_api_t *api)
{
    const bridge_query_t *q = test_get_query(api, nstore_with_a);
    const uint32_t *blocks = get_blocks(api);
    const uint32_t *span;
    uint64_t *who;
    uint32_t s = 0;
    uint32_t row;

    while (s < q->spans) {
        span = test_get_span(api, q, s, 1);
        who = (uint64_t *)(api->shared_base + blocks[span[0]]);
        row = 0;
        while (row < span[1]) {
            who[row] = nstore_ids[0];
            row++;
        }
        s++;
    }
}

TEST(scribble)
{
    overwrite_index(api);
    overwrite_names(api);
    return (0);
}

#endif
