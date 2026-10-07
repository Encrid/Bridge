/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/reference/flecs_ref_setup.c -- what the measures share to make a
** world ready, none of it timed: entities, their values, a query, and what
** the process holds.
*/

#include <stdlib.h>
#include <string.h>

#include "flecs_ref.h"

/*
** flecs_ref_make_ids
**
** Answers `count` new entities that carry `with`, or nothing if it is 0, in
** an array the caller frees.
*/
ecs_entity_t *flecs_ref_make_ids(ecs_world_t *w, ecs_entity_t with,
    int32_t count)
{
    ecs_entity_t *ids = malloc((size_t)count * sizeof(*ids));

    memcpy(ids, ecs_bulk_new_w_id(w, with, count),
        (size_t)count * sizeof(*ids));
    return (ids);
}

/*
** flecs_ref_set_ones
**
** Gives every entity of the measure its component, four bytes that hold 1.
*/
void flecs_ref_set_ones(const flecs_ref_t *r)
{
    const uint32_t one = 1;
    int32_t i = 0;

    while (i < r->count) {
        ecs_set_id(r->w, r->ids[i], r->id, sizeof(one), &one);
        i++;
    }
}

/*
** flecs_ref_make_query
**
** Answers a query for the entities that carry `first`, and `second` too
** unless it is 0.
*/
ecs_query_t *flecs_ref_make_query(ecs_world_t *w, ecs_entity_t first,
    ecs_entity_t second)
{
    ecs_query_desc_t desc = {};

    desc.terms[0].id = first;
    desc.terms[1].id = second;
    return (ecs_query_init(w, &desc));
}

#ifdef _WIN32

/*
** flecs_ref_read_held
**
** Answers the kilobytes the process holds.
*/
size_t flecs_ref_read_held(void)
{
    held_counters_t c = { .cb = sizeof(c) };

    if (!K32GetProcessMemoryInfo(GetCurrentProcess(), &c, sizeof(c)))
        return (0);
    return (c.now >> 10);
}

#else

/*
** flecs_ref_read_held
**
** Answers 0: what the process holds is asked on Windows alone.
*/
size_t flecs_ref_read_held(void)
{
    return (0);
}

#endif
