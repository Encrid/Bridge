/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/layout/layout_spawn.c -- the system of nlayout
** that gives X its values. The build whose V is 0 is the one a test loads
** first, so it alone spawns: every other build finds the values there.
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

/*
** spawn
**
** Spawns three entities with X: a 1000 and up, b 20 and up, arr 1 2 3 4
** and up, at 5 6 and up, ON. A singleton has one value and no entity, so
** that build gives the first of them once, with the id 0.
*/
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

/*
** spawn
**
** Does nothing.
*/
TEST(spawn)
{
    return (0);
}

#endif
