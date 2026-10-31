/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "storage_test.h"

constexpr uint32_t TIMED_MODULES = 1000;
constexpr uint32_t TIMED_ROUNDS = 20;
constexpr uint32_t TIMED_TABLES = 64;

static const char *const TIMED_PHASES[] = { "work", "churn", "idle" };

static int load_timed(bridge_world_t *w, uint32_t i)
{
    char identity[32];
    bridge_artifact_t a = storage_test_state(TIMED_PATH, 4, 4096, 4096);

    snprintf(identity, sizeof(identity), "tm:m%04u@0.1.0", i);
    a.cfg.identity = identity;
    return (bridge_load(w, &a));
}

static int make_timed(storage_test_t *t, uint32_t threads)
{
    uint32_t i = 0;

    (void)bridge_destroy(t->w);
    t->cfg.phases = TIMED_PHASES;
    t->cfg.phase_count = sizeof(TIMED_PHASES) / sizeof(*TIMED_PHASES);
    t->cfg.threads = threads;
    t->cfg.budget.modules = TIMED_MODULES + 1;
    t->cfg.budget.systems = 2 * TIMED_MODULES + 2;
    t->cfg.budget.tables = TIMED_TABLES;
    t->w = bridge_create(&t->cfg, storage_test_hear_nothing, t);
    while (t->w && i < TIMED_MODULES && !load_timed(t->w, i))
        i++;
    if (i == TIMED_MODULES)
        return (0);
    printf("FAIL: a thousand modules were not loaded\n");
    return (1);
}

static uint64_t time_load(storage_test_t *t, bool unload)
{
    uint64_t least = 0;
    uint64_t began;
    uint64_t now;

    for (uint32_t round = 0; round < TIMED_ROUNDS; round++) {
        began = os_get_now();
        if (load_timed(t->w, TIMED_MODULES))
            printf("FAIL: the module of the load was not loaded\n");
        now = os_get_now() - began;
        began = os_get_now();
        (void)bridge_unload(t->w, "tm:m1000@0.1.0");
        now = unload ? os_get_now() - began : now;
        least = !round || now < least ? now : least;
    }
    return (least);
}

static uint64_t time_barrier(storage_test_t *t, const char *phase)
{
    uint64_t least = 0;
    uint64_t now;

    for (uint32_t round = 0; round < TIMED_ROUNDS; round++) {
        now = storage_test_time_barrier(t, phase);
        least = !round || now < least ? now : least;
    }
    return (least);
}

int storage_test_measure_schedule(storage_test_t *t)
{
    if (make_timed(t, 1))
        return (1);
    printf("a phase of a thousand modules, on 1 thread   %7.1f us\n",
        (double)storage_test_time_least(t, "work", TIMED_ROUNDS) / 1e3);
    printf("a barrier of a thousand idle modules         %7.1f us\n",
        (double)time_barrier(t, "idle") / 1e3);
    printf("a barrier after a thousand queued changes    %7.1f us\n",
        (double)time_barrier(t, "churn") / 1e3);
    printf("a load beside a thousand modules             %7.1f us\n",
        (double)time_load(t, false) / 1e3);
    printf("an unload beside a thousand modules          %7.1f us\n",
        (double)time_load(t, true) / 1e3);
    if (make_timed(t, 6))
        return (1);
    printf("a phase of a thousand modules, on 6 threads  %7.1f us\n",
        (double)storage_test_time_least(t, "work", TIMED_ROUNDS) / 1e3);
    return (0);
}
