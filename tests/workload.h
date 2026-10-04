/* The workload both phase 1 tests run: everything that moves a row or resizes a
 * column. Table moves, swap-removes, deletes, a deferred batch, a shrink, and
 * one component deletion, which merges tables. */
#ifndef WORKLOAD_H
#define WORKLOAD_H

#include "flecs_confine.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { float x, y; } Position;
typedef struct { float x, y; } Velocity;
typedef struct { int32_t hp; } Health;

typedef struct {
    ecs_world_t *world;
    ecs_entity_t position, velocity, health;
    ecs_entity_t *e;        /* 0 once deleted */
    int32_t count;
    int32_t ops;
} workload_t;

/* Called after every batch, so a test can bring its own state up to date. */
typedef void (*workload_sync_fn)(workload_t *w);

static void workload_run(workload_t *w, int32_t count, workload_sync_fn sync)
{
    ecs_world_t *world = w->world;
    ECS_COMPONENT(world, Position);
    ECS_COMPONENT(world, Velocity);
    ECS_COMPONENT(world, Health);
    w->position = ecs_id(Position);
    w->velocity = ecs_id(Velocity);
    w->health = ecs_id(Health);
    w->count = count;
    w->e = calloc((size_t)count, sizeof(ecs_entity_t));
    w->ops = 0;
    int32_t i;

    for (i = 0; i < count; i ++) {                  /* grows, and resizes columns */
        w->e[i] = ecs_new(world);
        ecs_set(world, w->e[i], Position, {(float)i, (float)i});
        w->ops ++;
    }
    if (sync) sync(w);

    ecs_defer_begin(world);                         /* table moves, deferred */
    for (i = 0; i < count; i += 2) {
        ecs_set(world, w->e[i], Velocity, {1, 1});
        w->ops ++;
    }
    for (i = 0; i < count; i += 3) {
        ecs_set(world, w->e[i], Health, {100});
        w->ops ++;
    }
    ecs_defer_end(world);
    if (sync) sync(w);

    ecs_defer_begin(world);                         /* moves back, and swap-removes */
    for (i = 0; i < count; i += 5) {
        ecs_remove(world, w->e[i], Velocity);
        w->ops ++;
    }
    for (i = 0; i < count; i += 7) {
        ecs_delete(world, w->e[i]);
        w->e[i] = 0;
        w->ops ++;
    }
    ecs_defer_end(world);
    if (sync) sync(w);

    for (i = 1; i < count; i += 13) {               /* the same, immediately */
        if (w->e[i]) {
            ecs_delete(world, w->e[i]);
            w->e[i] = 0;
            w->ops ++;
        }
    }
    if (sync) sync(w);

    ecs_shrink(world);
    if (sync) sync(w);

    ecs_delete(world, ecs_id(Health));              /* merges every table that had it */
    w->health = 0;
    w->ops ++;
    if (sync) sync(w);
}

#endif
