/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/storage/storage_test_stress.c -- the stress run: four modules at
** once, each in its own namespace, a hundred rounds of work and check,
** eight million changes. Each module keeps its own account of what should
** be there and checks it. On one thread it must say the same as on four,
** word for word.
**
** schedule.md, *What a pass depends on*
*/

#include <stdio.h>

#include "storage_test.h"

constexpr uint32_t STRESSED = 4;
constexpr uint32_t STRESS_ROUNDS = 100;

static const char *const STRESS_IDENTITIES[STRESSED] = {
    "s1:stress@0.1.0", "s2:stress@0.1.0", "s3:stress@0.1.0", "s4:stress@0.1.0"
};

static const char *const STRESS_PATHS[STRESSED] = {
    NSTRESS1_PATH, NSTRESS2_PATH, NSTRESS3_PATH, NSTRESS4_PATH
};

static int load_stress(bridge_t *b)
{
    bridge_artifact_t a;
    uint32_t i = 0;

    while (i < STRESSED) {
        a = storage_test_state(STRESS_PATHS[i], 20'000, 2 << 20, 16 << 20);
        a.cfg.identity = STRESS_IDENTITIES[i];
        a.cfg.scratch = 1 << 20;
        if (bridge_load(b, &a))
            return (1);
        i++;
    }
    return (0);
}

static void run_stress(bridge_t *b)
{
    uint32_t round = 0;

    while (round < STRESS_ROUNDS) {
        if (bridge_run_phase(b, "work") || bridge_run_phase(b, "check"))
            printf("  REFUSED\n");
        round++;
    }
}

/*
** storage_test_stress
**
** The stress run, on as many threads as the case's deployment states.
*/
enum storage_test_end storage_test_stress(storage_test_t *t)
{
    if (load_stress(t->b)) {
        printf("FAIL: not loaded\n");
        return (STORAGE_TEST_FAILED);
    }
    run_stress(t->b);
    return (STORAGE_TEST_DONE);
}

/*
** storage_test_measure_stress
**
** The stress run on this many threads, timed, in a bridge of its own. Its
** records go nowhere: what they say is the stress case's to judge. Answers
** 1 if it could not load.
*/
int storage_test_measure_stress(storage_test_t *t, uint32_t threads)
{
    uint64_t began;

    (void)bridge_destroy(t->b);
    t->cfg.threads = threads;
    t->b = bridge_create(&t->cfg, storage_test_hear_nothing, t);
    if (!t->b || load_stress(t->b)) {
        printf("FAIL: not loaded\n");
        return (1);
    }
    began = os_get_now();
    run_stress(t->b);
    printf("the stress run, on %u thread%s            %7.0f ms\n", threads,
        threads > 1 ? "s" : " ", (double)(os_get_now() - began) / 1e6);
    storage_test_say_held(t->b);
    return (0);
}
