/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/native/ntoggle/ntoggle_find.c -- the system of ntoggle that
** reaches each entity by its id, and reads whether its T is on.
*/

#include "ntoggle.h"
#include "native/native_reach.h"

/*
** see_entity
**
** Counts the entity `e` is the record of if its T is on, and adds its
** value: the column through column_base, and the bits ahead of it.
*/
static void see_entity(const bridge_api_t *api, const bridge_entity_t *e,
    seen_t *seen)
{
    const bridge_header_t *h = bridge_native_get_header(api);
    const uint32_t *base = get_column_bases(api);
    uint32_t column = base[e->table * h->types + ntoggle_t];
    const uint64_t *bits;

    if (!column)
        return;
    bits = bridge_native_get_bits(api, column);
    if (!(bits[e->row >> 6] >> (e->row & 63) & 1))
        return;
    seen->total += ((const t_t *)(api->shared_base + column))[e->row].v;
    seen->on++;
}

/*
** find
**
** Reaches each entity by its id, and says how many are alive, how many of
** them have T on, and the sum of those.
*/
BRIDGE_NATIVE(find)
{
    const bridge_entity_t *index = get_entity_index(api);
    const bridge_entity_t *e;
    seen_t seen = {};
    uint32_t alive = 0;
    uint32_t i = 0;

    while (i < COUNT + ntoggle_more) {
        e = &index[(uint32_t)ntoggle_ids[i]];
        if (e->generation == (uint32_t)(ntoggle_ids[i] >> 32)) {
            alive++;
            see_entity(api, e, &seen);
        }
        i++;
    }
    say(api, "by id: alive, and of them on", alive, seen.on);
    say(api, "by id: the sum of those on", seen.total, 0);
    return (0);
}
