/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/threads.c -- two systems that nap, in two modules that
** share nothing: on two threads they run at once, and on one, in turn.
**
** threads.md, *Threads*
*/

#include <stdio.h>
#include <time.h>

#include "cases.h"

static double time_update(bridge_t *b)
{
    struct timespec t0;
    struct timespec t1;

    timespec_get(&t0, TIME_UTC);
    (void)bridge_run_phase(b, "update");
    timespec_get(&t1, TIME_UTC);
    return ((double)(t1.tv_sec - t0.tv_sec) * 1e3
        + (double)(t1.tv_nsec - t0.tv_nsec) / 1e6);
}

/*
** say_naps
**
** Says whether the two naps ran at once. At once is one beginning before
** the other ended, and nothing about how long either took: a sleep of a
** tenth of a second returns after two on some systems. The clock is in
** milliseconds, so two that follow each other may share one: not an
** overlap.
*/
static void say_naps(const test_counts_t *c, uint32_t threads)
{
    const uint32_t *began = c->began;
    const uint32_t *ended = c->ended;
    const char *how = "neither clearly";
    bool both = c->seen == 2 && c->ended_count == 2;
    bool overlap = both && began[0] < ended[1] && began[1] < ended[0];

    if (both && overlap && threads == 2)
        how = "they ran at once";
    if (both && !overlap && threads == 1)
        how = "they ran one after the other";
    printf("%s\n", how);
    if (both)
        printf("one from %u to %u ms, the other from %u to %u\n", 0,
            ended[0] - began[0], began[1] - began[0], ended[1] - began[0]);
}

static void nap_twice(host_t *h)
{
    const test_counts_t *c = test_get_counts(h);
    double ms;

    if (host_open(h))
        return;
    test_load_native(h, "test:sleep1@0.1.0", NSLEEP1_PATH, BRIDGE_NONE);
    test_load_native(h, "test:sleep2@0.1.0", NSLEEP2_PATH, BRIDGE_NONE);
    ms = time_update(h->b);
    printf("two naps took %.0f ms on %u thread(s)\n", ms,
        test_count_threads(c));
    say_naps(c, test_count_threads(c));
    host_close(h);
}

/*
** test_run_at_once
**
** Two threads: the naps overlap.
*/
void test_run_at_once(host_t *h)
{
    h->cfg.threads = 2;
    nap_twice(h);
}

/*
** test_run_in_turn
**
** One thread: one nap follows the other.
*/
void test_run_in_turn(host_t *h)
{
    nap_twice(h);
}
