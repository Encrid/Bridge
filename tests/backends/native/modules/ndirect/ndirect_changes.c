/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "ndirect.h"
#include "native_measured.h"

uint32_t ndirect_a;
uint32_t ndirect_d;
uint64_t ndirect_ids[COUNT + 2];

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

NATIVE_MEASURED BRIDGE_NATIVE(detach_b)
{
    uint32_t i = 0;

    while (i < COUNT) {
        api->detach(api, ndirect_ids[i], ndirect_d);
        i += STEP;
    }
    return (0);
}

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
