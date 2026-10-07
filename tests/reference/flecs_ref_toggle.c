/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/reference/flecs_ref_toggle.c -- a toggled component: a row
** switched, and the rows that are on read.
*/

#include <stdio.h>
#include <stdlib.h>

#include "flecs_ref.h"

/*
** time_switch
**
** Answers the nanoseconds it takes to switch every row, on for one draw in
** `one_in` and off for the others. The draw is timed with the switch.
** What the loop uses is copied out of `r` and `state` first, so that
** nothing is read back from them between two calls of flecs.
*/
static uint64_t time_switch(const flecs_ref_t *r, uint32_t *state,
    uint32_t one_in)
{
    ecs_world_t *w = r->w;
    const ecs_entity_t *ids = r->ids;
    ecs_entity_t id = r->id;
    uint32_t drawn = *state;
    int32_t i = 0;
    uint64_t t = read_clock();

    while (i < ROWS) {
        ecs_enable_id(w, ids[i], id, draw_random(&drawn) % one_in == 0);
        i++;
    }
    t = read_clock() - t;
    *state = drawn;
    return (t);
}

/*
** time_walk
**
** Answers the nanoseconds it takes to read the rows that are on. Adds what
** was read to `total`, and how many rows there were to `on`.
*/
static uint64_t time_walk(ecs_iter_t *it, uint32_t *total, uint32_t *on)
{
    const toggled_t *v;
    uint32_t sum = 0;
    int32_t i;
    uint64_t t = read_clock();

    while (ecs_query_next(it)) {
        v = ecs_field(it, toggled_t, 0);
        i = 0;
        while (i < it->count) {
            sum += v[i].v;
            i++;
        }
        *on += (uint32_t)it->count;
    }
    t = read_clock() - t;
    *total += sum;
    return (t);
}

/*
** time_switches
**
** Switches every row ROUNDS times, and answers the least time.
*/
static uint64_t time_switches(const flecs_ref_t *r, uint32_t *state,
    uint32_t one_in)
{
    uint64_t best = 0;
    uint64_t t;
    uint32_t round = 0;

    while (round < ROUNDS) {
        t = time_switch(r, state, one_in);
        if (!round || t < best)
            best = t;
        round++;
    }
    return (best);
}

/*
** time_walks
**
** Reads the rows that are on four times ROUNDS, and answers the least
** time. `on` is how many rows the last walk read.
*/
static uint64_t time_walks(const flecs_ref_t *r, ecs_query_t *q,
    uint32_t *total, uint32_t *on)
{
    ecs_iter_t it;
    uint64_t best = 0;
    uint64_t t;
    uint32_t round = 0;

    while (round < ROUNDS * 4) {
        it = ecs_query_iter(r->w, q);
        *on = 0;
        t = time_walk(&it, total, on);
        if (!round || t < best)
            best = t;
        round++;
    }
    return (best);
}

/*
** flecs_ref_measure_toggle
**
** Times a row of ROWS switched, and the rows that are on read, with one
** row in `one_in` on.
*/
void flecs_ref_measure_toggle(uint32_t *state, uint32_t one_in)
{
    flecs_ref_t r = { .w = ecs_mini(), .count = ROWS };
    ecs_query_t *q;
    uint64_t flip;
    uint64_t read;
    uint32_t sum = 0;
    uint32_t on = 0;

    ECS_COMPONENT(r.w, toggled_t);
    r.id = ecs_id(toggled_t);
    ecs_add_id(r.w, r.id, EcsCanToggle);
    r.ids = flecs_ref_make_ids(r.w, 0, ROWS);
    flecs_ref_set_ones(&r);
    q = flecs_ref_make_query(r.w, r.id, 0);
    flip = time_switches(&r, state, one_in);
    read = time_walks(&r, q, &sum, &on);
    printf("flecs, a toggle, one row in %2u on (%u of %d): the rows that "
        "are on read %6.1f us; a row switched %6.1f ns, with the draw (%u)\n",
        one_in, on, ROWS, (double)read / 1000.0, (double)flip / ROWS, sum & 1);
    free(r.ids);
    ecs_fini(r.w);
}
