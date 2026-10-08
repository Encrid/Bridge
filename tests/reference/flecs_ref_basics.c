/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/reference/flecs_ref_basics.c -- the lines of `storage measure`: an
** entity's first component, one it has, and its end.
*/

#include <stdio.h>
#include <stdlib.h>

#include "flecs_ref.h"

/*
** time_set
**
** Answers the nanoseconds it takes to set the component on every entity.
*/
static uint64_t time_set(const flecs_ref_t *r)
{
    ecs_world_t *w = r->w;
    const ecs_entity_t *ids = r->ids;
    ecs_entity_t id = r->id;
    small_t value = { 1 };
    int32_t count = r->count;
    int32_t i = 0;
    uint64_t t = read_clock();

    flecs_ref_begin(r);
    while (i < count) {
        ecs_set_id(w, ids[i], id, sizeof(value), &value);
        i++;
    }
    flecs_ref_end(r);
    return (read_clock() - t);
}

/*
** time_delete
**
** Answers the nanoseconds it takes to destroy every entity.
*/
static uint64_t time_delete(const flecs_ref_t *r)
{
    ecs_world_t *w = r->w;
    const ecs_entity_t *ids = r->ids;
    int32_t count = r->count;
    int32_t i = 0;
    uint64_t t = read_clock();

    flecs_ref_begin(r);
    while (i < count) {
        ecs_delete(w, ids[i]);
        i++;
    }
    flecs_ref_end(r);
    return (read_clock() - t);
}

/*
** flecs_ref_measure_basics
**
** Times a component set on 100,000 entities that have none, set again, and
** the entities destroyed.
*/
void flecs_ref_measure_basics(bool deferred)
{
    flecs_ref_t r = { .w = ecs_mini(), .count = 100'000 };
    uint64_t first;
    uint64_t again;
    uint64_t gone;

    ECS_COMPONENT(r.w, small_t);
    r.id = ecs_id(small_t);
    r.deferred = deferred;
    r.ids = flecs_ref_make_ids(r.w, 0, r.count);
    first = time_set(&r);
    again = time_set(&r);
    gone = time_delete(&r);
    printf("flecs, in tables, %s: an entity's first component %6.1f ns, one "
        "it has %6.1f ns, destroyed %6.1f ns\n",
        deferred ? "deferred" : "at once", (double)first / r.count,
        (double)again / r.count, (double)gone / r.count);
    free(r.ids);
    ecs_fini(r.w);
}
