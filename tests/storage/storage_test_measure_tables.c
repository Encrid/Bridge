/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/storage/storage_test_measure_tables.c -- what a barrier costs with
** ten thousand tables standing, and what making and emptying them costs.
*/

#include <stdio.h>

#include "storage_test.h"

/*
** time_idle
**
** Answers the least of a barrier with nothing to do, of one and then as
** many as a measurement has rounds.
*/
static uint64_t time_idle(storage_test_t *t)
{
    uint64_t least = storage_test_time_barrier(t, "never");
    uint64_t now;
    uint32_t round = 0;

    while (round < STORAGE_TEST_MEASURE_ROUNDS) {
        now = storage_test_time_barrier(t, "never");
        if (now < least)
            least = now;
        round++;
    }
    return (least);
}

/*
** measure_sets
**
** Every one of ten thousand sets filled at one barrier and emptied at the
** next: where a table that lives only while it has rows is made and
** released each time.
*/
static void measure_sets(storage_test_t *t)
{
    uint64_t filled = 0;
    uint64_t emptied = 0;
    uint64_t now;
    uint32_t round = 0;

    while (round < STORAGE_TEST_MEASURE_ROUNDS) {
        now = storage_test_time_barrier(t, "attach_b");
        if (!round || now < filled)
            filled = now;
        now = storage_test_time_barrier(t, "detach_b");
        if (!round || now < emptied)
            emptied = now;
        round++;
    }
    printf("%-38s %7.3f us a table\n", "ten thousand sets filled",
        (double)filled / 1000.0 / 10'000);
    printf("%-38s %7.3f us a table\n", "and emptied again",
        (double)emptied / 1000.0 / 10'000);
}

/*
** storage_test_measure_tables
**
** Ten thousand tables made, a barrier with them standing, ten thousand
** more, and the sets filled and emptied, in a deck of its own. Answers 1
** if the module could not be loaded.
*/
int storage_test_measure_tables(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(STORETAB_PATH, 10'000, 8 << 20,
        32 << 20);
    uint64_t now;

    if (storage_test_reopen(t, storage_test_hear, &a))
        return (1);
    (void)storage_test_time_barrier(t, "spawn");
    now = storage_test_time_barrier(t, "combine");
    printf("%-38s %7.3f us a table\n", "ten thousand tables made",
        (double)now / 1000.0 / 10'000);
    printf("%-38s %7.1f us\n", "a barrier with nothing to do, then",
        (double)time_idle(t) / 1000.0);
    now = storage_test_time_barrier(t, "attach_a");
    printf("%-38s %7.3f us a table\n", "ten thousand more, by one attach each",
        (double)now / 1000.0 / 10'000);
    measure_sets(t);
    storage_test_say_held(t->b);
    return (0);
}
