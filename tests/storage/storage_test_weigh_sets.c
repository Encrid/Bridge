/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/storage/storage_test_weigh_sets.c -- what a sparse set costs, and
** what a toggle costs: the change, and the read.
**
** storage.md, *A sparse set*
** storage.md, *A toggle*
*/

#include <stdio.h>

#include "storage_test.h"

static const storage_test_carried_t SETS[2] = {
    { SPARSEBIG_PATH, 4, 204'800 },
    { SPARSEFAT_PATH, 4096, 10'000 }
};

static const char *const SET_READS[2] = { "look", "look_b" };

static const storage_test_spread_t TOGGLED[] = {
    { TOGGLE1_PATH, 1 },
    { TOGGLE2_PATH, 2 },
    { TOGGLE16_PATH, 16 }
};

static const char *const TOGGLE_READS[2] = { "look_b", "look" };

/*
** weigh_set
**
** The change that was weighed in tables, of a component kept in a sparse
** set, on an entity that carries this much. Answers 1 if the module could
** not be loaded.
*/
static int weigh_set(storage_test_t *t, const storage_test_carried_t *row)
{
    storage_test_change_t c = {};
    bridge_artifact_t a = storage_test_state(row->path, row->count, 8 << 20,
        (int64_t)row->count * 4200);

    if (storage_test_reopen(t, storage_test_hear_counts, &a))
        return (1);
    (void)storage_test_time_barrier(t, "spawn");
    storage_test_time_give_take(t, &c);
    printf("a sparse set, an entity of %4u bytes: a component given %6.1f ns,"
        " taken %6.1f ns; and the calls %5.1f and %5.1f\n", row->bytes,
        (double)c.on / row->count, (double)c.off / row->count,
        (double)c.on_call / row->count, (double)c.off_call / row->count);
    return (0);
}

static void say_set_reads(storage_test_t *t, uint32_t count)
{
    uint64_t least[2] = {};

    storage_test_time_two(t, SET_READS, least);
    printf("    %u of its values walked %6.1f us; reached from a table's rows "
        "%6.1f us, %.3f ns a row\n", count, (double)least[0] / 1000.0,
        (double)least[1] / 1000.0, (double)least[1] / count);
}

/*
** storage_test_weigh_sets
**
** A sparse set, changed and read. It is read where the table's own rows
** are small: a row of 4 KB is a cache miss by itself, and that is the
** table's and says nothing of the set. Answers 1 if a module could not be
** loaded.
*/
int storage_test_weigh_sets(storage_test_t *t)
{
    if (weigh_set(t, &SETS[0]))
        return (1);
    say_set_reads(t, SETS[0].count);
    return (weigh_set(t, &SETS[1]));
}

/*
** weigh_toggle
**
** Rows read through their bits, where one row in this many is on: tested
** at every row, and found a word of bits at a time. Answers 1 if the
** module could not be loaded.
*/
static int weigh_toggle(storage_test_t *t, const storage_test_spread_t *row)
{
    bridge_artifact_t a = storage_test_state(row->path, 204'800, 8 << 20,
        128 << 20);
    uint64_t least[2] = {};

    if (storage_test_reopen(t, storage_test_hear_counts, &a))
        return (1);
    (void)storage_test_time_barrier(t, "spawn");
    (void)storage_test_time_systems(t, "flicker");
    storage_test_time_two(t, TOGGLE_READS, least);
    printf("a toggle, one row in %2u on: tested at every row %6.1f us, the "
        "rows that are on found a word at a time %6.1f us\n", row->share,
        (double)least[0] / 1000.0, (double)least[1] / 1000.0);
    return (0);
}

/*
** storage_test_weigh_toggles
**
** A toggle read, by the share of its rows that is on. Answers 1 if a
** module could not be loaded.
*/
int storage_test_weigh_toggles(storage_test_t *t)
{
    size_t k = 0;

    while (k < sizeof(TOGGLED) / sizeof(*TOGGLED)) {
        if (weigh_toggle(t, &TOGGLED[k]))
            return (1);
        k++;
    }
    return (0);
}
