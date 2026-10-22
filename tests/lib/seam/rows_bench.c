/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "cases.h"

void test_use_up_columns(host_t *h)
{
    h->cfg.budget.columns = (uint64_t)atoi(*h->arg ? h->arg : "0");
    if (host_open(h))
        return;
    test_load_mover(h);
    host_run_frames(h, 1);
    host_close(h);
}

static double run_passes(bridge_world_t *w)
{
    struct timespec t0;
    struct timespec t1;
    int pass = 0;

    timespec_get(&t0, TIME_UTC);
    while (pass < 50) {
        (void)bridge_run_phase(w, "update");
        pass++;
    }
    timespec_get(&t1, TIME_UTC);
    return ((double)(t1.tv_sec - t0.tv_sec) * 1e9
        + (double)(t1.tv_nsec - t0.tv_nsec));
}

static double time_passes(bridge_world_t *w)
{
    double best = 1e30;
    double ns;
    int round = 0;

    while (round < 20) {
        ns = run_passes(w);
        best = ns < best ? ns : best;
        round++;
    }
    return (best);
}

static void bench_build(host_t *h, const char *build, bool metered)
{
    bridge_world_t *w = bridge_create(&h->cfg, h->sink, nullptr);
    bridge_artifact_t a;

    test_state_module(&a, "game:rows@0.1.0", build);
    a.cfg.work_limit = metered ? 1'000'000'000 : BRIDGE_NO_LIMIT;
    a.cfg.init_timeout = BRIDGE_NO_LIMIT;
    a.cfg.reload_timeout = BRIDGE_NO_LIMIT;
    a.cfg.shutdown_timeout = BRIDGE_NO_LIMIT;
    a.cfg.spawns = 200'000;
    if (!w || bridge_load(w, &a)) {
        printf("bench: cannot load\n");
        exit(2);
    }
    printf("%-15s %-9s %.3f ns a row\n", build,
        metered ? "limit" : "no limit", time_passes(w) / (50.0 * 200'000.0));
    (void)bridge_destroy(w);
}

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
