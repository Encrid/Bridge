/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "cases.h"

static double time_update(bridge_world_t *w)
{
    struct timespec t0;
    struct timespec t1;

    timespec_get(&t0, TIME_UTC);
    (void)bridge_run_phase(w, "update");
    timespec_get(&t1, TIME_UTC);
    return ((double)(t1.tv_sec - t0.tv_sec) * 1e3
        + (double)(t1.tv_nsec - t0.tv_nsec) / 1e6);
}

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
    test_load_built(h, "test:sleep1@0.1.0", SLEEP1_PATH, BRIDGE_NONE);
    test_load_built(h, "test:sleep2@0.1.0", SLEEP2_PATH, BRIDGE_NONE);
    ms = time_update(h->w);
    printf("two naps took %.0f ms on %u thread(s)\n", ms,
        test_count_threads(c));
    say_naps(c, test_count_threads(c));
    host_close(h);
}

static void nap_in_one_module(host_t *h, const char *path)
{
    const test_counts_t *c = test_get_counts(h);

    if (host_open(h))
        return;
    test_load_built(h, "test:sleep@0.1.0", path, 1);
    (void)time_update(h->w);
    printf("two naps of one module on %u thread(s) of 2\n",
        test_count_threads(c));
    say_naps(c, test_count_threads(c));
    host_close(h);
}

static const bridge_order_t TEST_LANES_FIRST[3] = {
    { "lane1:lane:step", "test:bits:sum1" },
    { "lane2:lane:step", "test:bits:sum2" },
    { "lane3:lane:step", "test:bits:sum3" }
};

static const char *const TEST_LANE_VALUES[3] = {
    "lane1:Value", "lane2:Value", "lane3:Value"
};

static uint32_t load_lane(bridge_world_t *w, const char *identity,
    const char *file)
{
    bridge_artifact_t a;

    test_state_guarded(&a, identity, file);
    a.cfg.spawns = 128;
    return (bridge_load(w, &a) == 0);
}

static bool run_lanes(host_t *h, uint32_t threads, const char *third)
{
    bridge_world_t *w;
    bridge_artifact_t a;
    uint32_t loaded;
    int f = 0;

    h->cfg.threads = threads;
    h->cfg.order = TEST_LANES_FIRST;
    h->cfg.order_count = 3;
    w = bridge_create(&h->cfg, h->sink, h->ud);
    if (!w) {
        printf("bridge_create refused\n");
        return (false);
    }
    loaded = load_lane(w, "lane1:lane@0.1.0", "lane1");
    loaded += load_lane(w, "lane2:lane@0.1.0", "lane2");
    loaded += load_lane(w, "lane3:lane@0.1.0", third);
    test_state_guarded(&a, "test:bits@0.1.0", "bits");
    a.cfg.read = TEST_LANE_VALUES;
    a.cfg.read_count = 3;
    loaded += bridge_load(w, &a) == 0;
    printf("%u of 4 modules loaded on %u thread(s)\n", loaded, threads);
    while (f < 64) {
        (void)bridge_run_phase(w, "update");
        f++;
    }
    (void)bridge_destroy(w);
    return (true);
}

void test_lanes(host_t *h)
{
    test_counts_t *c = test_get_counts(h);
    bool same;

    c->minting = 0;
    if (!run_lanes(h, 1, "lane3"))
        return;
    c->minting = 1;
    if (!run_lanes(h, 4, h->arg))
        return;
    same = c->minted_count[0] == c->minted_count[1]
        && !memcmp(c->minted[0], c->minted[1], sizeof(c->minted[0]));
    printf("on 1 thread and on 4, %u frames give %s bits\n",
        c->minted_count[0] / 2, same ? "the same" : "DIFFERENT");
    printf("alive\n");
}

void test_nap(host_t *h)
{
    char *modules = nullptr;

    h->cfg.threads = (uint32_t)strtoul(h->arg, &modules, 10);
    if (!strcmp(modules, " heard"))
        nap_in_one_module(h, SLEEP_HEARD_PATH);
    else if (!strcmp(modules, " twice"))
        nap_in_one_module(h, SLEEP_TWICE_PATH);
    else
        nap_twice(h);
}
