/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/direct/direct_half.c -- the systems of ndirect
** that change one entity or two.
*/

#include "direct.h"

/*
** detach_b_half
**
** Takes D from the first entity that has it.
*/
TEST(detach_b_half)
{
    api->detach(api, ndirect_ids[0], ndirect_d);
    return (0);
}

/*
** value_b
**
** Gives a value to an entity that has D already.
*/
TEST(value_b)
{
    d_t value = { 900 };

    TEST_ATTACH(api, ndirect_ids[2 % COUNT], ndirect_d, value);
    return (0);
}

/*
** destroy_half
**
** Destroys an entity that has D, and one that has not.
*/
TEST(destroy_half)
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
TEST(respawn_half)
{
    a_t value = { COUNT };

    while (value.v < COUNT + 2) {
        ndirect_ids[value.v] = api->spawn(api);
        TEST_ATTACH(api, ndirect_ids[value.v], ndirect_a, value);
        value.v++;
    }
    return (0);
}
