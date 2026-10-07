/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/ntag/ntag_changes.c -- the systems of ntag that
** change something: the entities and their tags, and the bits of O.
*/

#include "ntag.h"

/*
** spawn
**
** Spawns six entities with A. M is given to every second one, P to the
** first two, O to every one, and R to the odd ones.
*/
BRIDGE_NATIVE(spawn)
{
    a_t value = {};

    while (value.v < COUNT) {
        ntag_ids[value.v] = api->spawn(api);
        BRIDGE_NATIVE_ATTACH(api, ntag_ids[value.v], ntag_a, value);
        if (value.v % 2 == 0)
            api->attach(api, ntag_ids[value.v], ntag_m, BRIDGE_ZEROED);
        if (value.v < 2)
            api->attach(api, ntag_ids[value.v], ntag_p, BRIDGE_ZEROED);
        api->attach(api, ntag_ids[value.v], ntag_o, BRIDGE_ZEROED);
        if (value.v % 2)
            api->attach(api, ntag_ids[value.v], ntag_r, BRIDGE_ZEROED);
        value.v++;
    }
    return (0);
}

/*
** turn_low_off
**
** Turns off, by a write of its bit, each row of a span whose A is less than
** three.
*/
static void turn_low_off(const a_t *column, uint64_t *bits, uint32_t count)
{
    uint32_t row = 0;

    while (row < count) {
        if (column[row].v < 3)
            bits[row >> 6] &= ~((uint64_t)1 << (row & 63));
        row++;
    }
}

/*
** flicker
**
** Turns O off for the first three entities, by a write of their bits.
*/
BRIDGE_NATIVE(flicker)
{
    const bridge_query_t *q = bridge_native_get_query(api, ntag_toggled);
    const uint32_t *span;
    uint32_t i = 0;

    while (i < q->spans) {
        span = bridge_native_get_span(api, q, i, 2);
        turn_low_off((const a_t *)(api->shared_base + span[2]),
            bridge_native_get_bits(api, span[3]), span[0]);
        i++;
    }
    return (0);
}

/*
** attach_a
**
** Gives W, the singleton.
*/
BRIDGE_NATIVE(attach_a)
{
    api->attach(api, 0, ntag_w, BRIDGE_ZEROED);
    return (0);
}

/*
** detach_b
**
** Takes M from every entity that has it, which leaves one table.
*/
BRIDGE_NATIVE(detach_b)
{
    uint32_t i = 0;

    while (i < COUNT) {
        api->detach(api, ntag_ids[i], ntag_m);
        i += 2;
    }
    return (0);
}
