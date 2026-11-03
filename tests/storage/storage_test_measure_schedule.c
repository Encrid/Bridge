/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "storage_test.h"

constexpr uint32_t TIMED_MODULES = 1000;
constexpr uint32_t TIMED_ROUNDS = 20;

static uint64_t time_load(storage_test_t *t, bool unload)
{
    char identity[32];
    const char *name = storage_test_name_timed(identity, sizeof(identity),
        TIMED_MODULES + 1);
    uint64_t least = 0;
    uint64_t began;
    uint64_t now;

    for (uint32_t round = 0; round < TIMED_ROUNDS; round++) {
        began = os_get_now();
        if (storage_test_load_timed(t->w, TIMED_MODULES + 1))
            printf("FAIL: the module of the load was not loaded\n");
        now = os_get_now() - began;
        began = os_get_now();
        (void)bridge_unload(t->w, name);
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
    if (storage_test_make_timed(t, TIMED_MODULES, 1))
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
    printf("a barrier after one changed table beside a thousand modules"
        " %7.1f us\n", (double)time_barrier(t, "spawn") / 1e3);
    if (storage_test_make_timed(t, TIMED_MODULES, 6))
        return (1);
    printf("a phase of a thousand modules, on 6 threads  %7.1f us\n",
        (double)storage_test_time_least(t, "work", TIMED_ROUNDS) / 1e3);
    return (0);
}
