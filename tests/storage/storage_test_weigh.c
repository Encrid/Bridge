/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "storage_test.h"

static const storage_test_carried_t CARRIED[] = {
    { STOREBIG_PATH, 4, 100'000 },
    { STOREFAT64_PATH, 64, 100'000 },
    { STOREFAT256_PATH, 256, 100'000 },
    { STOREFAT1K_PATH, 1024, 10'000 },
    { STOREFAT4K_PATH, 4096, 10'000 }
};

static const storage_test_spread_t SPREAD[] = {
    { FRAG1_PATH, 1 },
    { FRAG16_PATH, 16 },
    { FRAG256_PATH, 256 },
    { FRAG4096_PATH, 4096 }
};

static int weigh_carried(storage_test_t *t, const storage_test_carried_t *row)
{
    storage_test_change_t c = {};
    bridge_artifact_t a = storage_test_state(row->path, row->count, 1 << 20,
        (int64_t)row->count * 4200);

    if (storage_test_reopen(t, storage_test_hear, &a))
        return (1);
    (void)storage_test_time_barrier(t, "spawn");
    (void)storage_test_time_barrier(t, "attach_a");
    storage_test_time_attach_detach(t, &c);
    printf("an entity of %4u bytes: a component attached %6.1f ns, detached "
        "%6.1f ns\n", row->bytes, (double)c.on / row->count,
        (double)c.off / row->count);
    return (0);
}

static int weigh_spread(storage_test_t *t, const storage_test_spread_t *row)
{
    bridge_artifact_t a = storage_test_state(row->path, STORAGE_TEST_ROWS,
        8 << 20, 128 << 20);
    uint64_t best;

    if (storage_test_reopen(t, storage_test_hear_counts, &a))
        return (1);
    (void)storage_test_time_barrier(t, "spawn");
    best = storage_test_time_least(t, "look", STORAGE_TEST_WEIGH_ROUNDS * 4);
    if (t->said[0] != STORAGE_TEST_ROWS || t->said[1] != row->share) {
        printf("FAIL: it read %u rows in %u spans\n", t->said[0], t->said[1]);
        return (1);
    }
    printf("%u rows in %4u table%s: %6.1f us for all of them, %.3f ns a row\n",
        STORAGE_TEST_ROWS, row->share, row->share > 1 ? "s" : " ",
        (double)best / 1000.0, (double)best / STORAGE_TEST_ROWS);
    return (0);
}

static int weigh_tables(storage_test_t *t)
{
    size_t k = 0;

    while (k < sizeof(CARRIED) / sizeof(*CARRIED)) {
        if (weigh_carried(t, &CARRIED[k]))
            return (1);
        k++;
    }
    k = 0;
    while (k < sizeof(SPREAD) / sizeof(*SPREAD)) {
        if (weigh_spread(t, &SPREAD[k]))
            return (1);
        k++;
    }
    return (0);
}

enum storage_test_end storage_test_weigh(storage_test_t *t)
{
    t->cfg.budget.columns = 256 << 20;
    if (weigh_tables(t) || storage_test_weigh_sets(t))
        return (STORAGE_TEST_FAILED);
    if (storage_test_weigh_toggles(t) || storage_test_weigh_direct(t))
        return (STORAGE_TEST_FAILED);
    if (storage_test_weigh_tags(t) || storage_test_weigh_single(t))
        return (STORAGE_TEST_FAILED);
    if (storage_test_weigh_switch(t))
        return (STORAGE_TEST_FAILED);
    return (STORAGE_TEST_DONE);
}
