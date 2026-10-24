/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "direct.h"
#include "measured.h"

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

TEST(spawn)
{
    a_t value = {};

    while (value.v < COUNT) {
        ndirect_ids[value.v] = api->spawn(api);
        TEST_ATTACH(api, ndirect_ids[value.v], ndirect_a, value);
        value.v++;
    }
    mix_ids();
    return (0);
}

TEST_MEASURED TEST(attach_b)
{
    d_t value;
    uint32_t i = 0;

    while (i < COUNT) {
        value.v = 100 + i;
        TEST_ATTACH(api, ndirect_ids[i], ndirect_d, value);
        i += STEP;
    }
    return (0);
}

TEST_MEASURED TEST(detach_b)
{
    uint32_t i = 0;

    while (i < COUNT) {
        api->detach(api, ndirect_ids[i], ndirect_d);
        i += STEP;
    }
    return (0);
}

TEST(flicker)
{
    d_t value = { 7 };

    TEST_ATTACH(api, ndirect_ids[1], ndirect_d, value);
    api->detach(api, ndirect_ids[1], ndirect_d);
    return (0);
}

TEST(destroy)
{
    uint32_t i = 0;

    while (i < COUNT + 2) {
        if (ndirect_ids[i])
            api->destroy(api, ndirect_ids[i]);
        i++;
    }
    return (0);
}
