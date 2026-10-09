/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/store/store_one.c -- the changes the plain build
** of nstore asks for of one entity or two.
*/

#include "store.h"

#if !defined(BIG) && !defined(TABLES)
/*
** detach_b_absent
**
** Detaches B from an entity that does not carry it.
*/
TEST(detach_b_absent)
{
    api->detach(api, nstore_ids[COUNT - 1], nstore_b);
    return (0);
}

/*
** value_a
**
** Attaches A with a value, and writes over the bytes it handed after the
** call: it is the bytes at the call that are kept, so what is written
** afterwards must not be seen.
*/
TEST(value_a)
{
    a_t value = { 41 };
    uint32_t at = test_put(api, &value, sizeof(value));

    api->attach(api, nstore_ids[0], nstore_a, at);
    ((a_t *)(api->private_base + at))->v = 99;
    return (0);
}

/*
** flicker
**
** Changes one entity twice inside one barrier: B there and gone on the
** last, and A gone and back on the first.
*/
TEST(flicker)
{
    api->attach(api, nstore_ids[COUNT - 1], nstore_b, BRIDGE_ZEROED);
    api->detach(api, nstore_ids[COUNT - 1], nstore_b);
    api->detach(api, nstore_ids[0], nstore_a);
    api->attach(api, nstore_ids[0], nstore_a, BRIDGE_ZEROED);
    return (0);
}

/*
** touch_dead
**
** Asks for each change of an id that is gone: each is passed over.
*/
TEST(touch_dead)
{
    api->attach(api, nstore_old[COUNT - 1], nstore_a, BRIDGE_ZEROED);
    api->detach(api, nstore_old[COUNT - 1], nstore_a);
    api->destroy(api, nstore_old[COUNT - 1]);
    return (0);
}

#endif
