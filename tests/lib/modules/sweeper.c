/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_module.h"
#include "idle.h"
#include "say.h"

TEST(sweep)
{
    const bridge_header_t *h = (const bridge_header_t *)api->private_base;
    const bridge_entity_t *index = (const bridge_entity_t *)(api->shared_base
        + h->entity_index);
    uint32_t swept = 0;
    uint32_t i = 1;

    while (i < h->entities) {
        if (index[i].generation && index[i].generation != UINT32_MAX
            && !index[i].table) {
            api->destroy(api, (uint64_t)index[i].generation << 32 | i);
            swept++;
        }
        i++;
    }
    say(api, "swept", swept, 0);
    return (0);
}

TEST(init)
{
    int none = 0;

    TEST_ADD_SYSTEM(api, "sweep", "update", &none, 0, nullptr, 0, nullptr, 0);
    return (0);
}
