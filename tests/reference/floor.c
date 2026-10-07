/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/reference/floor.c -- what reaching a value by entity id costs with
** nothing in the way: a C array with a cell for every entity, the id's
** generation tested and a bit that says the entity has a value. No ECS
** ships a direct-indexed storage layout to measure ours beside, so this is
** the least it could cost.
**
** Built and run by tools/reference.py. Not part of the library, and not
** built by build.py.
**
** stage11.md, *Beside the best there is*
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "floor.h"
#include "reference.h"

constexpr uint32_t ROWS = 204'800;
constexpr uint32_t ROUNDS = 32;

/*
** sum_reached
**
** Answers the sum of the values ROWS ids reach: an id whose generation is
** not its entity's reaches nothing, and neither does an entity with no
** value. It is not inlined, so that what is timed is this loop alone, and
** the arrays are taken out of `f` before it, so that it reads nothing of
** `f` at a row.
*/
[[gnu::noinline]]
static uint32_t sum_reached(const floor_t *f, const uint64_t *ids)
{
    const floor_entity_t *index = f->index;
    const uint32_t *cells = f->cells;
    const uint64_t *given = f->given;
    uint32_t total = 0;
    uint32_t e;
    uint32_t i = 0;

    while (i < ROWS) {
        e = (uint32_t)ids[i];
        if (index[e].generation == (uint32_t)(ids[i] >> 32)
            && (given[e >> 6] >> (e & 63) & 1))
            total += cells[e];
        i++;
    }
    return (total);
}

/*
** time_reached
**
** Answers the nanoseconds it takes to reach the values of ROWS ids. The sum
** is written where the compiler cannot drop it.
*/
static uint64_t time_reached(const floor_t *f, const uint64_t *ids)
{
    [[maybe_unused]] volatile uint32_t sink = 0;
    uint64_t t = read_clock();

    sink = sum_reached(f, ids);
    return (read_clock() - t);
}

/*
** fill_floor
**
** Makes the arrays, with ROWS entities that each have a value, and their
** ids twice: `near` in the order they lie, `far` drawn in no order.
*/
static void fill_floor(floor_t *f, uint64_t *near, uint64_t *far)
{
    uint32_t state = 2463534242u;
    uint32_t i = 0;

    f->index = malloc((ROWS + 1) * sizeof(*f->index));
    f->cells = malloc((ROWS + 1) * 4);
    f->given = malloc((ROWS / 64 + 2) * 8);
    memset(f->given, 0xff, (ROWS / 64 + 2) * 8);
    while (i < ROWS) {
        f->index[i + 1] = (floor_entity_t){ 1, i, 1 };
        f->cells[i + 1] = 1;
        near[i] = (uint64_t)1 << 32 | (i + 1);
        i++;
    }
    i = 0;
    while (i < ROWS) {
        far[i] = (uint64_t)1 << 32 | (draw_random(&state) % ROWS + 1);
        i++;
    }
}

/*
** time_rounds
**
** Reaches the values by the ids in the order they lie, and then in no
** order, ROUNDS times, and keeps the least time of each.
*/
static void time_rounds(const floor_t *f, const uint64_t *near,
    const uint64_t *far, uint64_t *best)
{
    uint64_t t;
    uint32_t round = 0;

    while (round < ROUNDS) {
        t = time_reached(f, near);
        if (!round || t < best[0])
            best[0] = t;
        t = time_reached(f, far);
        if (!round || t < best[1])
            best[1] = t;
        round++;
    }
}

/*
** main
**
** Times ROWS values reached by id, in the order they lie and in none, and
** says the least time of each.
*/
int main(void)
{
    floor_t f = {};
    uint64_t *near = malloc(ROWS * 8);
    uint64_t *far = malloc(ROWS * 8);
    uint64_t best[2] = {};

    fill_floor(&f, near, far);
    time_rounds(&f, near, far, best);
    printf("a C array by entity: %u values reached by id, in the order they "
        "lie %6.3f ns each, in no order %6.3f ns each\n", ROWS,
        (double)best[0] / ROWS, (double)best[1] / ROWS);
    return (0);
}
