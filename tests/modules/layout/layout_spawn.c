/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "layout.h"

#if V == 0

static void set_value(x_t *value, uint32_t i)
{
    uint32_t k = 0;

    value->a = 1000 + i;
    value->b = (uint16_t)(20 + i);
    value->at.x = (uint16_t)(5 + i);
    value->at.y = (uint16_t)(6 + i);
    value->mode = ON;
    while (k < 4) {
        value->arr[k] = (uint8_t)(1 + k + i);
        k++;
    }
}

TEST(spawn)
{
    x_t value = {};
    uint64_t id = 0;
    uint32_t i = 0;

    while (i < (KEPT == 5 ? 1u : 3u)) {
        set_value(&value, i);
        if (KEPT != 5)
            id = api->spawn(api);
        TEST_ATTACH(api, id, x_slot, value);
        i++;
    }
    return (0);
}

#else

TEST(spawn)
{
    return (0);
}

#endif
