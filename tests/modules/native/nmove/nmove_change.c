/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/nmove/nmove_change.c -- the systems of nmove that
** change who carries X: the spawn, the two detaches, and the toggle.
*/

#include "nmove.h"

/*
** spawn
**
** Spawns six entities with A, and gives X to every second one: 100, 102,
** 104. A singleton has one value and no entity, so that build gives it
** once, with the id 0: 104.
*/
BRIDGE_NATIVE(spawn)
{
    a_t first = {};
    x_t second = {};
    uint32_t i = 0;

    while (i < COUNT) {
        first.v = i;
        second.v = 100 + i;
        ids[i] = api->spawn(api);
        BRIDGE_NATIVE_ATTACH(api, ids[i], a_slot, first);
        if (KEPT == 5 && i == 4)
            BRIDGE_NATIVE_ATTACH(api, 0, x_slot, second);
        if (KEPT != 5 && i % 2 == 0)
            BRIDGE_NATIVE_ATTACH(api, ids[i], x_slot, second);
        i++;
    }
    return (0);
}

/*
** detach_b
**
** Takes X from two of the three that carry it, so that one entity does. It
** does nothing to a singleton.
*/
BRIDGE_NATIVE(detach_b)
{
    if (KEPT == 5)
        return (0);
    api->detach(api, ids[0], x_slot);
    api->detach(api, ids[2], x_slot);
    return (0);
}

/*
** detach_b_half
**
** Takes X from all three that carry it, so that none does. It does nothing
** to a singleton.
*/
BRIDGE_NATIVE(detach_b_half)
{
    uint32_t i = 0;

    if (KEPT == 5)
        return (0);
    while (i < COUNT) {
        api->detach(api, ids[i], x_slot);
        i += 2;
    }
    return (0);
}

static void turn_off(const bridge_api_t *api, const uint32_t *span)
{
    const x_t *column = (const x_t *)(api->shared_base + span[2]);
    uint64_t *bits = bridge_native_get_bits(api, span[2]);
    uint32_t r = 0;

    while (r < span[0]) {
        if (column[r].v == 102)
            bits[r >> 6] &= ~((uint64_t)1 << (r & 63));
        r++;
    }
}

/*
** flicker
**
** Turns off the row whose value is 102, where X is kept as a toggle. It
** does nothing in any other build.
*/
BRIDGE_NATIVE(flicker)
{
    const bridge_query_t *q;
    uint32_t i = 0;

    if (KEPT != 3)
        return (0);
    q = bridge_native_get_query(api, reading);
    while (i < q->spans) {
        turn_off(api, bridge_native_get_span(api, q, i, 1));
        i++;
    }
    return (0);
}
