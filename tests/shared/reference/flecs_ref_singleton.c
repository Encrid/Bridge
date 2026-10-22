/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "flecs_ref.h"

constexpr uint32_t READS = 1'000'000;

static uint64_t time_reads(const ecs_world_t *w, ecs_entity_t id,
    uint32_t *total)
{
    const globals_t *value;
    uint32_t sum = 0;
    uint32_t i = 0;
    uint64_t t = read_clock();

    while (i < READS) {
        value = ecs_get_id(w, id, id);
        sum += value->gravity;
        i++;
    }
    t = read_clock() - t;
    *total += sum;
    return (t);
}

void flecs_ref_measure_singleton(void)
{
    ecs_world_t *w = ecs_mini();
    uint64_t best = 0;
    uint64_t t;
    uint32_t total = 0;
    uint32_t round = 0;

    ECS_COMPONENT(w, globals_t);
    ecs_add_id(w, ecs_id(globals_t), EcsSingleton);
    ecs_singleton_set(w, globals_t, { 10, 1 });
    while (round < ROUNDS * 4) {
        t = time_reads(w, ecs_id(globals_t), &total);
        if (!round || t < best)
            best = t;
        round++;
    }
    printf("flecs, a singleton read: %6.3f ns each time (%u)\n",
        (double)best / READS, total & 1);
    ecs_fini(w);
}
