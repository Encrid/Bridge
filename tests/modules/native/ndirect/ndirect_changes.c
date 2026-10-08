/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/ndirect/ndirect_changes.c -- the systems of ndirect
** that change every entity, or every one that is given D.
*/

#include "ndirect.h"
#include "native/native_measured.h"

uint32_t ndirect_a;
uint32_t ndirect_d;
uint64_t ndirect_ids[COUNT + 2];

/*
** mix_ids
**
** Fills the ids that `far` reaches by: as many as there are entities, each
** the id of an entity a generator drew.
*/
static void mix_ids(void)
{
    uint32_t state = 2463534242u;
    uint32_t i = 0;

    while (i < COUNT) {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        ndirect_mixed[i] = ndirect_ids[state % COUNT];
        i++;
    }
}

/*
** spawn
**
** Spawns the entities, each with A, and draws the ids in no order.
*/
BRIDGE_NATIVE(spawn)
{
    a_t value = {};

    while (value.v < COUNT) {
        ndirect_ids[value.v] = api->spawn(api);
        BRIDGE_NATIVE_ATTACH(api, ndirect_ids[value.v], ndirect_a, value);
        value.v++;
    }
    mix_ids();
    return (0);
}

/*
** attach_b
**
** Gives D to every second entity, or in a build for a measurement to every
** one, each with a value of its own.
*/
NATIVE_MEASURED BRIDGE_NATIVE(attach_b)
{
    d_t value;
    uint32_t i = 0;

    while (i < COUNT) {
        value.v = 100 + i;
        BRIDGE_NATIVE_ATTACH(api, ndirect_ids[i], ndirect_d, value);
        i += STEP;
    }
    return (0);
}

/*
** detach_b
**
** Takes D from each entity `attach_b` gave it to.
*/
NATIVE_MEASURED BRIDGE_NATIVE(detach_b)
{
    uint32_t i = 0;

    while (i < COUNT) {
        api->detach(api, ndirect_ids[i], ndirect_d);
        i += STEP;
    }
    return (0);
}

/*
** destroy
**
** Destroys every entity there is, the two of `respawn_half` among them.
*/
BRIDGE_NATIVE(destroy)
{
    uint32_t i = 0;

    while (i < COUNT + 2) {
        if (ndirect_ids[i])
            api->destroy(api, ndirect_ids[i]);
        i++;
    }
    return (0);
}
