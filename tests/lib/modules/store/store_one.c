/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "store.h"

#if !defined(BIG) && !defined(TABLES)
TEST(detach_b_absent)
{
    api->detach(api, nstore_ids[COUNT - 1], nstore_b);
    return (0);
}

TEST(value_a)
{
    a_t value = { 41 };
    uint32_t at = test_put(api, &value, sizeof(value));

    api->attach(api, nstore_ids[0], nstore_a, at);
    ((a_t *)(api->private_base + at))->v = 99;
    return (0);
}

TEST(flicker)
{
    api->attach(api, nstore_ids[COUNT - 1], nstore_b, BRIDGE_ZEROED);
    api->detach(api, nstore_ids[COUNT - 1], nstore_b);
    api->detach(api, nstore_ids[0], nstore_a);
    api->attach(api, nstore_ids[0], nstore_a, BRIDGE_ZEROED);
    return (0);
}

TEST(attach_three)
{
    uint32_t i = 0;

    while (i < COUNT) {
        api->attach(api, nstore_ids[i], nstore_a, BRIDGE_ZEROED);
        if (i < COUNT / 2)
            api->attach(api, nstore_ids[i], nstore_b, BRIDGE_ZEROED);
        api->attach(api, nstore_ids[i], nstore_c, BRIDGE_ZEROED);
        i++;
    }
    return (0);
}

static uint32_t count_taken(void)
{
    uint32_t taken = 0;
    uint32_t i = COUNT / 2;
    uint32_t k;

    while (i < COUNT) {
        k = COUNT / 2;
        while (k < COUNT) {
            taken += (uint32_t)nstore_ids[i] == (uint32_t)nstore_old[k];
            k++;
        }
        i++;
    }
    return (taken);
}

TEST(stale)
{
    uint32_t i = COUNT / 2;

    say(api, "stale ids whose index a live id took", count_taken(), 0);
    while (i < COUNT) {
        api->attach(api, nstore_ids[i], nstore_b, BRIDGE_ZEROED);
        api->attach(api, nstore_old[i], nstore_b, BRIDGE_ZEROED);
        i++;
    }
    return (0);
}

TEST(touch_dead)
{
    api->attach(api, nstore_old[COUNT - 1], nstore_a, BRIDGE_ZEROED);
    api->detach(api, nstore_old[COUNT - 1], nstore_a);
    api->destroy(api, nstore_old[COUNT - 1]);
    return (0);
}

#endif
