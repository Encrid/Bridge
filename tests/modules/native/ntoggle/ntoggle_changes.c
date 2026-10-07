/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/native/ntoggle/ntoggle_changes.c -- the systems of ntoggle
** that ask for a structural change: entities made and destroyed, and
** components given.
*/

#include "ntoggle.h"

uint32_t ntoggle_a;
uint32_t ntoggle_b;
uint32_t ntoggle_t;
uint64_t ntoggle_ids[2 * COUNT];
uint32_t ntoggle_more;

/*
** spawn_entities
**
** Spawns as many entities as the build has, from the place `from` of the
** ids, each with A and T: a toggle is on when it is given.
*/
static void spawn_entities(const bridge_api_t *api, uint32_t from)
{
    a_t first;
    t_t second;
    uint32_t i = from;

    while (i < from + COUNT) {
        first.v = i;
        second.v = 10 + i;
        ntoggle_ids[i] = api->spawn(api);
        BRIDGE_NATIVE_ATTACH(api, ntoggle_ids[i], ntoggle_a, first);
        BRIDGE_NATIVE_ATTACH(api, ntoggle_ids[i], ntoggle_t, second);
        i++;
    }
}

/*
** spawn
**
** Spawns the entities.
*/
BRIDGE_NATIVE(spawn)
{
    spawn_entities(api, 0);
    return (0);
}

/*
** respawn_half
**
** Spawns as many entities again, so that the table is moved to grow.
*/
BRIDGE_NATIVE(respawn_half)
{
    spawn_entities(api, COUNT);
    ntoggle_more = COUNT;
    return (0);
}

/*
** attach_b
**
** Gives B to the first half of the entities: they move to another table,
** as they were.
*/
BRIDGE_NATIVE(attach_b)
{
    b_t value = {};

    while (value.v < COUNT / 2) {
        BRIDGE_NATIVE_ATTACH(api, ntoggle_ids[value.v], ntoggle_b, value);
        value.v++;
    }
    return (0);
}

/*
** destroy_half
**
** Destroys the first entity and the last: another row takes each place.
*/
BRIDGE_NATIVE(destroy_half)
{
    api->destroy(api, ntoggle_ids[0]);
    api->destroy(api, ntoggle_ids[COUNT - 1]);
    return (0);
}
