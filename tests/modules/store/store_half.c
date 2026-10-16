/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "store.h"

#if !defined(BIG) && !defined(TABLES)
TEST(attach_b_half)
{
    uint32_t i = 0;

    while (i < COUNT / 2) {
        api->attach(api, nstore_ids[i], nstore_b, BRIDGE_ZEROED);
        i++;
    }
    return (0);
}

TEST(detach_b_half)
{
    uint32_t i = 0;

    while (i < COUNT / 2) {
        api->detach(api, nstore_ids[i], nstore_b);
        i++;
    }
    return (0);
}

TEST(attach_a_half)
{
    uint32_t i = COUNT / 2;

    while (i < COUNT) {
        api->attach(api, nstore_ids[i], nstore_a, BRIDGE_ZEROED);
        i++;
    }
    return (0);
}

TEST(value_b)
{
    b_t value;
    uint32_t i = 0;

    while (i < COUNT / 2) {
        value.v = 70 + i;
        TEST_ATTACH(api, nstore_ids[i], nstore_b, value);
        i++;
    }
    return (0);
}

#endif
