/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/ndirect/ndirect_half.c -- the systems of ndirect
** that change one entity or two.
*/

#include "ndirect.h"

/*
** detach_b_half
**
** Takes D from the first entity that has it.
*/
BRIDGE_NATIVE(detach_b_half)
{
    api->detach(api, ndirect_ids[0], ndirect_d);
    return (0);
}

/*
** value_b
**
** Gives a value to an entity that has D already.
*/
BRIDGE_NATIVE(value_b)
{
    d_t value = { 900 };

    BRIDGE_NATIVE_ATTACH(api, ndirect_ids[2 % COUNT], ndirect_d, value);
    return (0);
}

/*
** destroy_half
**
** Destroys an entity that has D, and one that has not.
*/
BRIDGE_NATIVE(destroy_half)
{
    api->destroy(api, ndirect_ids[2 % COUNT]);
    api->destroy(api, ndirect_ids[1 % COUNT]);
    return (0);
}

/*
** respawn_half
**
** Spawns two more entities, with A alone: they are given the places of the
** two that went, and none of what was there.
*/
BRIDGE_NATIVE(respawn_half)
{
    a_t value = { COUNT };

    while (value.v < COUNT + 2) {
        ndirect_ids[value.v] = api->spawn(api);
        BRIDGE_NATIVE_ATTACH(api, ndirect_ids[value.v], ndirect_a, value);
        value.v++;
    }
    return (0);
}
