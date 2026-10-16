/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "storage_test.h"

static void keep_least(uint64_t *least, uint64_t now, bool first)
{
    if (first || now < *least)
        *least = now;
}

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
