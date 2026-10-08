/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/storage/storage_test_time.c -- a phase, timed: its systems, and
** its barrier. A change is the call that asks for it and the barrier that
** applies it, and a library that applies a change at the call has one
** figure where this has two.
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

/*
** storage_test_time_barrier
**
** Runs the systems of one phase and then its barrier, each timed. Answers
** the nanoseconds of the barrier, and leaves those of the systems in
** `t->called`.
*/
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

/*
** storage_test_time_systems
**
** Runs the systems of one phase, timed, and then its barrier. Answers the
** nanoseconds of the systems.
*/
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

/*
** storage_test_time_least
**
** Answers the least of the systems of a phase, timed `rounds` times.
*/
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

/*
** storage_test_time_two
**
** Leaves in `least` the least of the systems of two phases, run one after
** the other four times as often as a change is weighed.
*/
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
