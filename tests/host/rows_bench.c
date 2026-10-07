/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/host/rows_bench.c -- the accessor's row test, against a build
** without it, with the count that a work limit adds and without; and a
** column region the mover's rows use up.
**
** validation.md, *Always on*
*/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "cases.h"

/*
** test_use_up_columns
**
** A column region of as many bytes as the case's argument says: storage
** cannot hold what the mover spawns.
*/
void test_use_up_columns(host_t *h)
{
    h->cfg.budget.columns = (uint64_t)atoi(*h->arg ? h->arg : "0");
    if (host_open(h))
        return;
    test_load_mover(h);
    host_run_frames(h, 1);
    host_close(h);
}

/*
** run_passes
**
** Answers the nanoseconds fifty passes of "update" take.
*/
static double run_passes(bridge_t *b)
{
    struct timespec t0;
    struct timespec t1;
    int pass = 0;

    timespec_get(&t0, TIME_UTC);
    while (pass < 50) {
        (void)bridge_run_phase(b, "update");
        pass++;
    }
    timespec_get(&t1, TIME_UTC);
    return ((double)(t1.tv_sec - t0.tv_sec) * 1e9
        + (double)(t1.tv_nsec - t0.tv_nsec));
}

/*
** time_passes
**
** Answers the nanoseconds fifty passes of "update" take, the best of
** twenty.
*/
static double time_passes(bridge_t *b)
{
    double best = 1e30;
    double ns;
    int round = 0;

    while (round < 20) {
        ns = run_passes(b);
        best = ns < best ? ns : best;
        round++;
    }
    return (best);
}

/*
** bench_build
**
** Times one build of the rows module, under a work limit or under none, and
** says the nanoseconds a row.
*/
static void bench_build(host_t *h, const char *build, bool metered)
{
    bridge_t *b = bridge_create(&h->cfg, h->sink, nullptr);
    bridge_artifact_t a;

    test_state_module(&a, "game:rows@0.1.0", build);
    a.cfg.work_limit = metered ? 1'000'000'000 : BRIDGE_NO_LIMIT;
    a.cfg.init_timeout = BRIDGE_NO_LIMIT;
    a.cfg.reload_timeout = BRIDGE_NO_LIMIT;
    a.cfg.shutdown_timeout = BRIDGE_NO_LIMIT;
    a.cfg.spawns = 200'000;
    if (!b || bridge_load(b, &a)) {
        printf("bench: cannot load\n");
        exit(2);
    }
    printf("%-15s %-9s %.3f ns a row\n", build,
        metered ? "limit" : "no limit", time_passes(b) / (50.0 * 200'000.0));
    (void)bridge_destroy(b);
}

/*
** test_bench
**
** Times the two builds of the rows module, each with a work limit and
** without.
*/
void test_bench(host_t *h)
{
    h->cfg.budget.entities = 262'144;
    h->cfg.budget.columns = 64 << 20;
    bench_build(h, "rows", true);
    bench_build(h, "rows", false);
    bench_build(h, "rows-unchecked", true);
    bench_build(h, "rows-unchecked", false);
    printf("alive\n");
}
