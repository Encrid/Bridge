/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "store.h"
#include "measured.h"

uint32_t nstore_a;
uint32_t nstore_b;
uint32_t nstore_c;
uint64_t nstore_ids[COUNT];

TEST_MEASURED TEST(attach_a)
{
    uint32_t i = 0;

    while (i < COUNT) {
        api->attach(api, nstore_ids[i], nstore_a, BRIDGE_ZEROED);
        i++;
    }
    return (0);
}

TEST_MEASURED TEST(attach_b)
{
    uint32_t i = 0;

    while (i < COUNT) {
        api->attach(api, nstore_ids[i], nstore_b, BRIDGE_ZEROED);
        i++;
    }
    return (0);
}

TEST_MEASURED TEST(detach_b)
{
    uint32_t i = 0;

    while (i < COUNT) {
        api->detach(api, nstore_ids[i], nstore_b);
        i++;
    }
    return (0);
}

TEST_MEASURED TEST(destroy)
{
    uint32_t i = 0;

    while (i < COUNT) {
        api->destroy(api, nstore_ids[i]);
        i++;
    }
    return (0);
}

TEST_MEASURED TEST(attach_both)
{
    uint32_t i = 0;

    while (i < COUNT) {
        api->attach(api, nstore_ids[i], nstore_a, BRIDGE_ZEROED);
        api->attach(api, nstore_ids[i], nstore_b, BRIDGE_ZEROED);
        i++;
    }
    return (0);
}
