/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "ndirect.h"

BRIDGE_NATIVE(detach_b_half)
{
    api->detach(api, ndirect_ids[0], ndirect_d);
    return (0);
}

BRIDGE_NATIVE(value_b)
{
    d_t value = { 900 };

    BRIDGE_NATIVE_ATTACH(api, ndirect_ids[2 % COUNT], ndirect_d, value);
    return (0);
}

BRIDGE_NATIVE(destroy_half)
{
    api->destroy(api, ndirect_ids[2 % COUNT]);
    api->destroy(api, ndirect_ids[1 % COUNT]);
    return (0);
}

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
