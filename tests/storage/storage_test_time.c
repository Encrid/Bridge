/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "storage_test.h"

static uint32_t find_phase(const storage_test_t *t, const char *phase)
{
    uint32_t i = 0;

    while (i < t->cfg.phase_count && strcmp(t->cfg.phases[i], phase))
        i++;
    return (i);
}

uint64_t storage_test_time_barrier(storage_test_t *t, const char *phase)
{
    bridge_t *b = t->b;
    uint32_t i = find_phase(t, phase);
    uint64_t began = os_get_now();

    schedule_run(b, i);
    t->called = os_get_now() - began;
    began = os_get_now();
    block_barrier(b);
    return (os_get_now() - began);
}

uint64_t storage_test_time_systems(storage_test_t *t, const char *phase)
{
    bridge_t *b = t->b;
    uint32_t i = find_phase(t, phase);
    uint64_t spent = os_get_now();

    schedule_run(b, i);
    spent = os_get_now() - spent;
    block_barrier(b);
    return (spent);
}

uint64_t storage_test_time_least(storage_test_t *t, const char *phase,
    uint32_t rounds)
{
    uint64_t least = 0;
    uint64_t now;
    uint32_t round = 0;

    while (round < rounds) {
        now = storage_test_time_systems(t, phase);
        if (!round || now < least)
            least = now;
        round++;
    }
    return (least);
}

void storage_test_time_two(storage_test_t *t, const char *const *phases,
    uint64_t *least)
{
    uint64_t now;
    uint32_t round = 0;

    while (round < STORAGE_TEST_WEIGH_ROUNDS * 4) {
        now = storage_test_time_systems(t, phases[0]);
        if (!round || now < least[0])
            least[0] = now;
        now = storage_test_time_systems(t, phases[1]);
        if (!round || now < least[1])
            least[1] = now;
        round++;
    }
}
