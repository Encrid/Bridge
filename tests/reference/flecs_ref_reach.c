/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>

#include "flecs_ref.h"

static uint64_t time_reach(const flecs_ref_t *r, const ecs_entity_t *ids,
    uint32_t *total)
{
    const ecs_world_t *w = r->w;
    ecs_entity_t id = r->id;
    const small_t *value;
    uint32_t sum = 0;
    int32_t i = 0;
    uint64_t t = read_clock();

    while (i < ROWS) {
        value = ecs_get_id(w, ids[i], id);
        sum += value->v;
        i++;
    }
    t = read_clock() - t;
    *total += sum;
    return (t);
}

static void time_rounds(const flecs_ref_t *r, const ecs_entity_t *mixed,
    uint64_t *best, uint32_t *total)
{
    uint64_t t;
    uint32_t round = 0;

    while (round < ROUNDS * 4) {
        t = time_reach(r, r->ids, total);
        if (!round || t < best[0])
            best[0] = t;
        t = time_reach(r, mixed, total);
        if (!round || t < best[1])
            best[1] = t;
        round++;
    }
}

static ecs_entity_t *open_reach(flecs_ref_t *r, uint32_t *state, bool sparse)
{
    ecs_entity_t *mixed;
    int32_t i = 0;

    ECS_COMPONENT(r->w, small_t);
    r->id = ecs_id(small_t);
    mixed = malloc((size_t)ROWS * sizeof(*mixed));
    if (sparse) {
        ecs_add_id(r->w, r->id, EcsSparse);
        ecs_add_id(r->w, r->id, EcsDontFragment);
    }
    r->ids = flecs_ref_make_ids(r->w, 0, ROWS);
    flecs_ref_set_ones(r);
    while (i < ROWS) {
        mixed[i] = r->ids[draw_random(state) % ROWS];
        i++;
    }
    return (mixed);
}

void flecs_ref_measure_reach(uint32_t *state, bool sparse)
{
    flecs_ref_t r = { .w = ecs_mini(), .count = ROWS };
    ecs_entity_t *mixed = open_reach(&r, state, sparse);
    uint64_t best[2] = {};
    uint32_t total = 0;

    time_rounds(&r, mixed, best, &total);
    printf("flecs, %s: %d values reached by id, in the order they lie "
        "%6.3f ns each, in no order %6.3f ns each (%u)\n",
        sparse ? "sparse" : "in tables", ROWS, (double)best[0] / ROWS,
        (double)best[1] / ROWS, total & 1);
    free(r.ids);
    free(mixed);
    ecs_fini(r.w);
}
