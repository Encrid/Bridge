/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "direct.h"

TEST(detach_b_half)
{
    api->detach(api, ndirect_ids[0], ndirect_d);
    return (0);
}

TEST(value_b)
{
    d_t value = { 900 };

    TEST_ATTACH(api, ndirect_ids[2 % COUNT], ndirect_d, value);
    return (0);
}

TEST(destroy_half)
{
    api->destroy(api, ndirect_ids[2 % COUNT]);
    api->destroy(api, ndirect_ids[1 % COUNT]);
    return (0);
}

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
