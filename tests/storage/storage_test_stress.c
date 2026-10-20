/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "storage_test.h"

constexpr uint32_t STRESSED = 4;
constexpr uint32_t STRESS_ROUNDS = 100;

static const char *const STRESS_IDENTITIES[STRESSED] = {
    "s1:stress@0.1.0", "s2:stress@0.1.0", "s3:stress@0.1.0", "s4:stress@0.1.0"
};

static const char *const STRESS_PATHS[STRESSED] = {
    STRESS1_PATH, STRESS2_PATH, STRESS3_PATH, STRESS4_PATH
};

static int load_stress(bridge_world_t *w)
{
    bridge_artifact_t a;
    uint32_t i = 0;

    while (i < STRESSED) {
        a = storage_test_state(STRESS_PATHS[i], 20'000, 2 << 20, 16 << 20);
        a.cfg.identity = STRESS_IDENTITIES[i];
        a.cfg.scratch = 1 << 20;
        if (bridge_load(w, &a))
            return (1);
        i++;
    }
    return (0);
}

static void run_stress(bridge_world_t *w)
{
    uint32_t round = 0;

    while (round < STRESS_ROUNDS) {
        if (bridge_run_phase(w, "work") || bridge_run_phase(w, "check"))
            printf("  REFUSED\n");
        round++;
    }
}

enum storage_test_end storage_test_stress(storage_test_t *t)
{
    if (load_stress(t->w)) {
        printf("FAIL: not loaded\n");
        return (STORAGE_TEST_FAILED);
    }
    run_stress(t->w);
    return (STORAGE_TEST_DONE);
}

int storage_test_measure_stress(storage_test_t *t, uint32_t threads)
{
    uint64_t began;

    (void)bridge_destroy(t->w);
    t->cfg.threads = threads;
    t->w = bridge_create(&t->cfg, storage_test_hear_nothing, t);
    if (!t->w || load_stress(t->w)) {
        printf("FAIL: not loaded\n");
        return (1);
    }
    began = os_get_now();
    run_stress(t->w);
    printf("the stress run, on %u thread%s            %7.0f ms\n", threads,
        threads > 1 ? "s" : " ", (double)(os_get_now() - began) / 1e6);
    storage_test_say_held(t->w);
    return (0);
}
