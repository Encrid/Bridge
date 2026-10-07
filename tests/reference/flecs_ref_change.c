/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/reference/flecs_ref_change.c -- a four-byte component given to, and
** taken from, an entity that carries four bytes or 4,096.
*/

#include <stdio.h>
#include <stdlib.h>

#include "flecs_ref.h"

/*
** time_give
**
** Answers the nanoseconds it takes to give every entity the component.
*/
static uint64_t time_give(const flecs_ref_t *r)
{
    ecs_world_t *w = r->w;
    const ecs_entity_t *ids = r->ids;
    ecs_entity_t id = r->id;
    sparse_t value;
    int32_t count = r->count;
    int32_t i = 0;
    uint64_t t = read_clock();

    flecs_ref_begin(r);
    while (i < count) {
        value = (sparse_t){ 100 + (uint32_t)i };
        ecs_set_id(w, ids[i], id, sizeof(value), &value);
        i++;
    }
    flecs_ref_end(r);
    return (read_clock() - t);
}

/*
** time_take
**
** Answers the nanoseconds it takes to take the component from every entity.
*/
static uint64_t time_take(const flecs_ref_t *r)
{
    ecs_world_t *w = r->w;
    const ecs_entity_t *ids = r->ids;
    ecs_entity_t id = r->id;
    int32_t count = r->count;
    int32_t i = 0;
    uint64_t t = read_clock();

    flecs_ref_begin(r);
    while (i < count) {
        ecs_remove_id(w, ids[i], id);
        i++;
    }
    flecs_ref_end(r);
    return (read_clock() - t);
}

/*
** time_rounds
**
** Gives and takes ROUNDS times, and keeps the least time of each.
*/
static void time_rounds(const flecs_ref_t *r, uint64_t *on, uint64_t *off)
{
    uint64_t t;
    uint32_t round = 0;

    while (round < ROUNDS) {
        t = time_give(r);
        if (!round || t < *on)
            *on = t;
        t = time_take(r);
        if (!round || t < *off)
            *off = t;
        round++;
    }
}

/*
** open_change
**
** Makes the components, the one that is given sparse and not fragmenting if
** that is asked, and the entities, which carry the fat component or the
** small one.
*/
static void open_change(flecs_ref_t *r, bool fat, bool sparse)
{
    ECS_COMPONENT(r->w, small_t);
    ECS_COMPONENT(r->w, fat_t);
    ECS_COMPONENT(r->w, sparse_t);
    r->id = ecs_id(sparse_t);
    if (sparse) {
        ecs_add_id(r->w, r->id, EcsSparse);
        ecs_add_id(r->w, r->id, EcsDontFragment);
    }
    r->ids = flecs_ref_make_ids(r->w,
        fat ? ecs_id(fat_t) : ecs_id(small_t), r->count);
}

/*
** flecs_ref_measure_change
**
** Times the component given and taken, on 10,000 entities if they are fat
** and on ROWS if not.
*/
void flecs_ref_measure_change(bool fat, bool sparse, bool deferred)
{
    flecs_ref_t r = { .w = ecs_mini(), .count = fat ? 10'000 : ROWS };
    const char *where = sparse ? "sparse and not fragmenting" : "in tables";
    const char *when = deferred ? "deferred" : "at once";
    uint64_t on = 0;
    uint64_t off = 0;

    r.deferred = deferred;
    open_change(&r, fat, sparse);
    time_rounds(&r, &on, &off);
    printf("flecs, %s, %s, an entity of %4u bytes: a component given "
        "%6.1f ns, taken %6.1f ns\n", where, when, fat ? 4096u : 4u,
        (double)on / r.count, (double)off / r.count);
    free(r.ids);
    ecs_fini(r.w);
}
