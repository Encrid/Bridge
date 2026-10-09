/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/store/store_scribble.c -- what storage publishes
** in the shared block, written over by a module that should not.
*/

#include "store.h"
#include "reach.h"

#if !defined(BIG) && !defined(TABLES)
/*
** overwrite_index
**
** Makes the index record of every entity of the module that of the first.
*/
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

/*
** overwrite_names
**
** Makes every row of the entity array of every table with A name the first
** entity.
*/
static void overwrite_names(const test_api_t *api)
{
    const bridge_query_t *q = test_get_query(api, nstore_with_a);
    const uint32_t *span;
    uint64_t *who;
    uint32_t s = 0;
    uint32_t row;

    while (s < q->spans) {
        span = test_get_span(api, q, s, 1);
        who = (uint64_t *)(api->shared_base + span[1]);
        row = 0;
        while (row < span[0]) {
            who[row] = nstore_ids[0];
            row++;
        }
        s++;
    }
}

/*
** scribble
**
** Writes over the index record of every one of the module's entities, and
** over the entity array of every table with A. Each is made to name the
** first entity, which is alive: a storage that read either back would act
** on it.
*/
TEST(scribble)
{
    overwrite_index(api);
    overwrite_names(api);
    return (0);
}

#endif
