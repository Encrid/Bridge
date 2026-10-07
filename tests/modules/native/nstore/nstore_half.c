/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/nstore/nstore_half.c -- the changes the plain build
** of nstore asks for of half of its entities.
*/

#include "nstore.h"

#if !defined(BIG) && !defined(TABLES)
/*
** attach_b_half
**
** Attaches B to the first half of the entities.
*/
BRIDGE_NATIVE(attach_b_half)
{
    uint32_t i = 0;

    while (i < COUNT / 2) {
        api->attach(api, nstore_ids[i], nstore_b, BRIDGE_ZEROED);
        i++;
    }
    return (0);
}

/*
** detach_b_half
**
** Detaches B from the first half of the entities.
*/
BRIDGE_NATIVE(detach_b_half)
{
    uint32_t i = 0;

    while (i < COUNT / 2) {
        api->detach(api, nstore_ids[i], nstore_b);
        i++;
    }
    return (0);
}

/*
** attach_a_half
**
** Attaches A to the second half of the entities.
*/
BRIDGE_NATIVE(attach_a_half)
{
    uint32_t i = COUNT / 2;

    while (i < COUNT) {
        api->attach(api, nstore_ids[i], nstore_a, BRIDGE_ZEROED);
        i++;
    }
    return (0);
}

/*
** value_b
**
** Attaches B to the first half of the entities, each with a value of its
** own: for a column that has to be where it was after the one before it is
** taken away, which a query for B alone then sees.
*/
BRIDGE_NATIVE(value_b)
{
    b_t value;
    uint32_t i = 0;

    while (i < COUNT / 2) {
        value.v = 70 + i;
        BRIDGE_NATIVE_ATTACH(api, nstore_ids[i], nstore_b, value);
        i++;
    }
    return (0);
}

#endif
