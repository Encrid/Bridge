/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/storage/storage_test_time_changes.c -- a component given to every
** entity and taken from it, over and over, and the least each cost.
*/

#include "storage_test.h"

static void keep_least(uint64_t *least, uint64_t now, bool first)
{
    if (first || now < *least)
        *least = now;
}

/*
** storage_test_time_attach_detach
**
** Leaves in `c` the least of the barriers of attach_b and of detach_b, over
** the rounds a change is weighed.
*/
void storage_test_time_attach_detach(storage_test_t *t,
    storage_test_change_t *c)
{
    uint64_t now;
    uint32_t round = 0;

    while (round < STORAGE_TEST_WEIGH_ROUNDS) {
        now = storage_test_time_barrier(t, "attach_b");
        keep_least(&c->on, now, !round);
        now = storage_test_time_barrier(t, "detach_b");
        keep_least(&c->off, now, !round);
        round++;
    }
}

static void time_take(storage_test_t *t, storage_test_change_t *c, bool first)
{
    uint64_t now = storage_test_time_barrier(t, "detach_b");

    keep_least(&c->off, now, first);
    keep_least(&c->off_call, t->called, first);
}

/*
** storage_test_time_give_take
**
** Leaves in `c` the least of the barriers of attach_b and of detach_b, and
** of the module's own calls in each, over the rounds a change is weighed.
** The last round takes nothing: the component is left on, for what reads
** it afterwards.
*/
void storage_test_time_give_take(storage_test_t *t, storage_test_change_t *c)
{
    uint64_t now;
    uint32_t round = 0;

    while (round < STORAGE_TEST_WEIGH_ROUNDS) {
        now = storage_test_time_barrier(t, "attach_b");
        keep_least(&c->on, now, !round);
        keep_least(&c->on_call, t->called, !round);
        if (round + 1 < STORAGE_TEST_WEIGH_ROUNDS)
            time_take(t, c, !round);
        round++;
    }
}
