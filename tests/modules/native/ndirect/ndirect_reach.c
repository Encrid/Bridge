/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/ndirect/ndirect_reach.c -- the systems of ndirect
** that reach D by ids that are held, and walk nothing.
*/

#include "ndirect.h"
#include "native/native_measured.h"
#include "native/native_reach.h"

uint64_t ndirect_mixed[COUNT];

/*
** count_reached
**
** Reaches each of `count` entities by an id that is held: the generation of
** its record of the index, its bit, and its cell. Counts in `sum` those
** that are alive, those of them given D, and the sum of their D.
*/
static void count_reached(const bridge_api_t *api, const uint64_t *held,
    uint32_t count, reached_t *sum)
{
    const bridge_entity_t *index = get_entity_index(api);
    uint32_t cells = bridge_native_get_cells(api, ndirect_d);
    const uint64_t *given = bridge_native_get_bits(api, cells);
    const d_t *cell = (const d_t *)(api->shared_base + cells);
    uint64_t id;
    uint32_t e;
    uint32_t i = 0;

    while (i < count) {
        id = held[i];
        e = (uint32_t)id;
        i++;
        if (!id || index[e].generation != (uint32_t)(id >> 32))
            continue;
        sum->alive++;
        if (given[e >> 6] >> (e & 63) & 1) {
            sum->total += cell[e].v;
            sum->found++;
        }
    }
}

/*
** reach
**
** Reaches each of `count` entities by an id that is held, and says how many
** are alive, how many of them were given D, and the sum of their D.
*/
NATIVE_MEASURED static int reach(const bridge_api_t *api, const uint64_t *held,
    uint32_t count)
{
    reached_t sum = {};

    count_reached(api, held, count, &sum);
    say(api, "by id: alive, and of them given D", sum.alive, sum.found);
    say(api, "by id: the sum of D", sum.total, 0);
    return (0);
}

/*
** near
**
** Reaches every entity by its id, in the order they lie.
*/
BRIDGE_NATIVE(near)
{
    return (reach(api, ndirect_ids, COUNT + 2));
}

/*
** far
**
** Reaches entities by ids that are in no order.
*/
BRIDGE_NATIVE(far)
{
    return (reach(api, ndirect_mixed, COUNT));
}
