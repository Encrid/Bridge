/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "floor.h"
#include "reference.h"

constexpr uint32_t ROWS = 204'800;
constexpr uint32_t ROUNDS = 32;

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

static uint64_t time_reached(const floor_t *f, const uint64_t *ids)
{
    [[maybe_unused]] volatile uint32_t sink = 0;
    uint64_t t = read_clock();

    sink = sum_reached(f, ids);
    return (read_clock() - t);
}

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
