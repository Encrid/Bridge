/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/nstore/nstore_changes.c -- the changes every build
** of nstore asks for, of every entity, one phase each. A component is
** attached as zero bytes, so that a hundred thousand of them need no
** scratch arena.
*/

#include "nstore.h"
#include "native/native_measured.h"

uint32_t nstore_a;
uint32_t nstore_b;
uint64_t nstore_ids[COUNT];

/*
** attach_a
**
** Attaches A to every entity.
*/
NATIVE_MEASURED BRIDGE_NATIVE(attach_a)
{
    uint32_t i = 0;

    while (i < COUNT) {
        api->attach(api, nstore_ids[i], nstore_a, BRIDGE_ZEROED);
        i++;
    }
    return (0);
}

/*
** attach_b
**
** Attaches B to every entity.
*/
NATIVE_MEASURED BRIDGE_NATIVE(attach_b)
{
    uint32_t i = 0;

    while (i < COUNT) {
        api->attach(api, nstore_ids[i], nstore_b, BRIDGE_ZEROED);
        i++;
    }
    return (0);
}

/*
** detach_b
**
** Detaches B from every entity.
*/
NATIVE_MEASURED BRIDGE_NATIVE(detach_b)
{
    uint32_t i = 0;

    while (i < COUNT) {
        api->detach(api, nstore_ids[i], nstore_b);
        i++;
    }
    return (0);
}

/*
** destroy
**
** Destroys every entity.
*/
NATIVE_MEASURED BRIDGE_NATIVE(destroy)
{
    uint32_t i = 0;

    while (i < COUNT) {
        api->destroy(api, nstore_ids[i]);
        i++;
    }
    return (0);
}

/*
** attach_both
**
** Attaches two components in one barrier, to entities that carry neither.
*/
NATIVE_MEASURED BRIDGE_NATIVE(attach_both)
{
    uint32_t i = 0;

    while (i < COUNT) {
        api->attach(api, nstore_ids[i], nstore_a, BRIDGE_ZEROED);
        api->attach(api, nstore_ids[i], nstore_b, BRIDGE_ZEROED);
        i++;
    }
    return (0);
}
