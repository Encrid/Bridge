/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/storage/storage_test_measure.c -- nanoseconds for each kind of
** change, at a barrier: a measurement, which passes by running.
*/

#include <stdio.h>

#include "storage_test.h"

constexpr uint32_t MEASURE_COUNT = 100'000;

/*
** The phases of one round, in order. The second spawn and the second
** destroy only make and undo what attach_both needs, and have no line.
*/
static const storage_test_op_t OPS[STORAGE_TEST_OPS] = {
    { "spawn", "an id handed out" },
    { "attach_a", "attach, the entity's first component" },
    { "attach_a", "attach, a component it has" },
    { "attach_b", "attach, a second component" },
    { "detach_b", "detach" },
    { "destroy", "destroy" },
    { "spawn", nullptr },
    { "attach_both", "attach, two components in one barrier" },
    { "destroy", nullptr },
    { "never", "a barrier with nothing to do" }
};

static void time_round(storage_test_t *t, storage_test_times_t *m, bool first)
{
    uint64_t now;
    uint32_t i = 0;

    while (i < STORAGE_TEST_OPS) {
        now = storage_test_time_barrier(t, OPS[i].phase);
        if (first)
            m->first[i] = now;
        if (first || now < m->best[i])
            m->best[i] = now;
        if (first || t->called < m->calls[i])
            m->calls[i] = t->called;
        i++;
    }
}

static void say_ops(const storage_test_times_t *m)
{
    constexpr uint32_t last = STORAGE_TEST_OPS - 1;
    uint32_t i = 0;

    printf("%u entities, the best of %u rounds, and the first, where tables "
        "are new; and what the module's own call cost\n", MEASURE_COUNT,
        STORAGE_TEST_MEASURE_ROUNDS);
    while (i < last) {
        if (OPS[i].what)
            printf("%-38s %7.1f ns each   first %7.1f   call %5.1f\n",
                OPS[i].what, (double)m->best[i] / MEASURE_COUNT,
                (double)m->first[i] / MEASURE_COUNT,
                (double)m->calls[i] / MEASURE_COUNT);
        i++;
    }
    printf("%-38s %7.0f ns\n", OPS[last].what, (double)m->best[last]);
}

/*
** measure_changes
**
** Each kind of change, of a hundred thousand entities in the deck main
** made. What is held is said with every entity carrying both components
** again: a destroyed one holds nothing in either. Answers 1 if the module
** could not be loaded.
*/
static int measure_changes(storage_test_t *t)
{
    storage_test_times_t m = {};
    bridge_artifact_t a = storage_test_state(NSTOREBIG_PATH, MEASURE_COUNT,
        1 << 20, 64 << 20);
    uint32_t round = 0;

    if (storage_test_load(t, &a))
        return (1);
    while (round < STORAGE_TEST_MEASURE_ROUNDS) {
        time_round(t, &m, !round);
        round++;
    }
    say_ops(&m);
    (void)storage_test_time_barrier(t, "spawn");
    (void)storage_test_time_barrier(t, "attach_both");
    storage_test_say_held(t->b);
    return (0);
}

/*
** storage_test_measure
**
** Each kind of change; then the same deck with ten thousand tables
** standing; then the stress run, on four threads and on one: eight million
** changes and their checks.
*/
enum storage_test_end storage_test_measure(storage_test_t *t)
{
    if (measure_changes(t) || storage_test_measure_tables(t))
        return (STORAGE_TEST_FAILED);
    if (storage_test_measure_stress(t, 4) || storage_test_measure_stress(t, 1))
        return (STORAGE_TEST_FAILED);
    return (STORAGE_TEST_DONE);
}
