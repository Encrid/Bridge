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

TEST(touch_dead)
{
    api->attach(api, nstore_old[COUNT - 1], nstore_a, BRIDGE_ZEROED);
    api->detach(api, nstore_old[COUNT - 1], nstore_a);
    api->destroy(api, nstore_old[COUNT - 1]);
    return (0);
}

#endif
