/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/reference/flecs_ref_reads.c -- read: a table's column; a sparse
** component alone; and the sparse one beside the column. A query's iterator
** is made before its walk, and is not timed.
*/

#include <stdio.h>
#include <stdlib.h>

#include "flecs_ref.h"

/*
** time_column
**
** Answers the nanoseconds it takes to read a table's column, and adds what
** was read to `total`.
*/
static uint64_t time_column(ecs_iter_t *it, uint32_t *total)
{
    const small_t *a;
    uint32_t sum = 0;
    int32_t i;
    uint64_t t = read_clock();

    while (ecs_query_next(it)) {
        a = ecs_field(it, small_t, 0);
        i = 0;
        while (i < it->count) {
            sum += a[i].v;
            i++;
        }
    }
    t = read_clock() - t;
    *total += sum;
    return (t);
}

/*
** time_alone
**
** Answers the nanoseconds it takes to walk a sparse component alone, and
** adds what was read to `total`.
*/
static uint64_t time_alone(ecs_iter_t *it, uint32_t *total)
{
    uint32_t sum = 0;
    int32_t i;
    uint64_t t = read_clock();

    while (ecs_query_next(it)) {
        i = 0;
        while (i < it->count) {
            sum += ecs_field_at(it, sparse_t, 0, i)->v;
            i++;
        }
    }
    t = read_clock() - t;
    *total += sum;
    return (t);
}

/*
** time_beside
**
** Answers the nanoseconds it takes to read a table's column and the sparse
** component of the same entities, and adds what was read to `total`.
*/
static uint64_t time_beside(ecs_iter_t *it, uint32_t *total)
{
    const small_t *a;
    uint32_t sum = 0;
    int32_t i;
    uint64_t t = read_clock();

    while (ecs_query_next(it)) {
        a = ecs_field(it, small_t, 0);
        i = 0;
        while (i < it->count) {
            sum += a[i].v + ecs_field_at(it, sparse_t, 1, i)->v;
            i++;
        }
    }
    t = read_clock() - t;
    *total += sum;
    return (t);
}

/*
** time_reads
**
** Walks the three queries in turn, four times ROUNDS, and keeps the least
** time of each.
*/
static void time_reads(ecs_world_t *w, ecs_query_t **q, uint64_t *best,
    uint32_t *total)
{
    ecs_iter_t it;
    uint64_t t;
    uint32_t round = 0;

    while (round < ROUNDS * 4) {
        it = ecs_query_iter(w, q[0]);
        t = time_column(&it, total);
        if (!round || t < best[0])
            best[0] = t;
        it = ecs_query_iter(w, q[1]);
        t = time_alone(&it, total);
        if (!round || t < best[1])
            best[1] = t;
        it = ecs_query_iter(w, q[2]);
        t = time_beside(&it, total);
        if (!round || t < best[2])
            best[2] = t;
        round++;
    }
}

/*
** flecs_ref_measure_reads
**
** Times the three reads over ROWS entities.
*/
void flecs_ref_measure_reads(void)
{
    flecs_ref_t r = { .w = ecs_mini(), .count = ROWS };
    ecs_query_t *q[3];
    uint64_t best[3] = {};
    uint32_t total = 0;

    flecs_ref_open_reads(&r, q);
    time_reads(r.w, q, best, &total);
    printf("flecs, %d rows: a table's column read %6.1f us; a sparse "
        "component walked alone %6.1f us; read beside the column %6.1f us, "
        "%.3f ns a row (%u)\n", ROWS, (double)best[0] / 1000.0,
        (double)best[1] / 1000.0, (double)best[2] / 1000.0,
        (double)best[2] / ROWS, total & 1);
    free(r.ids);
    ecs_fini(r.w);
}
