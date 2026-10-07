/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/native/nweigh/nweigh_follow.c -- the systems of nweigh that
** reach a value by the id of its entity, as a reference is followed.
*/

#include "nweigh.h"
#include "native/native_measured.h"
#include "native/native_reach.h"

uint32_t nweigh_value;
uint32_t nweigh_linked;

/*
** add_followed
**
** Follows the link of every row of a span to the value of the entity it
** names, the far one or the near one, through the index and column_base.
** Answers `total` and the values it reached: an entity that is gone, or
** that has no value, adds nothing.
*/
static uint32_t add_followed(const bridge_api_t *api, const uint32_t *span,
    bool far, uint32_t total)
{
    const bridge_header_t *h = bridge_native_get_header(api);
    const bridge_entity_t *index = get_entity_index(api);
    const uint32_t *base = get_column_bases(api);
    const l_t *column = (const l_t *)(api->shared_base + span[2]);
    const bridge_entity_t *e;
    uint64_t id;
    uint32_t at;
    uint32_t r = 0;

    while (r < span[0]) {
        id = far ? column[r].far : column[r].near;
        e = &index[(uint32_t)id];
        at = 0;
        if (e->generation == (uint32_t)(id >> 32))
            at = base[e->table * h->types + nweigh_value];
        if (at)
            total += ((const v_t *)(api->shared_base + at))[e->row].v;
        r++;
    }
    return (total);
}

/*
** follow
**
** Follows every row's link, the far one or the near one, and says the sum
** of the values it reached.
*/
NATIVE_MEASURED static int follow(const bridge_api_t *api, bool far)
{
    const bridge_query_t *q = bridge_native_get_query(api, nweigh_linked);
    uint32_t total = 0;
    uint32_t s = 0;

    while (s < q->spans) {
        total = add_followed(api, bridge_native_get_span(api, q, s, 1), far,
            total);
        s++;
    }
    say_read(api, total, q->spans);
    return (0);
}

/*
** near
**
** Reaches each value by an id, the ids in the order their entities lie.
*/
BRIDGE_NATIVE(near)
{
    return (follow(api, false));
}

/*
** far
**
** Reaches each value by an id, the ids in no order.
*/
BRIDGE_NATIVE(far)
{
    return (follow(api, true));
}
