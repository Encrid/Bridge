/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "storage_test.h"

static const char *const ID_READS[2] = { "near", "far" };

int storage_test_weigh_direct(storage_test_t *t)
{
    storage_test_change_t c = {};
    bridge_artifact_t a = storage_test_state(DIRECTBIG_PATH, 204'802,
        8 << 20, 128 << 20);
    uint64_t least[2] = {};

    if (storage_test_reopen(t, storage_test_hear_counts, &a))
        return (1);
    (void)storage_test_time_barrier(t, "spawn");
    storage_test_time_give_take(t, &c);
    storage_test_time_two(t, ID_READS, least);
    printf("direct-indexed: a component given %6.1f ns, taken %6.1f ns; and "
        "the calls %5.1f and %5.1f\n", (double)c.on / 204'800,
        (double)c.off / 204'800, (double)c.on_call / 204'800,
        (double)c.off_call / 204'800);
    printf("    204800 values reached by id, in the order they lie %6.3f ns "
        "each, in no order %6.3f ns each\n", (double)least[0] / 204'800,
        (double)least[1] / 204'800);
    return (0);
}

static int use_columns(storage_test_t *t, const char *path, uint32_t *used)
{
    bridge_artifact_t a = storage_test_state(path, STORAGE_TEST_ROWS, 8 << 20,
        128 << 20);

    if (storage_test_reopen(t, storage_test_hear_counts, &a))
        return (1);
    (void)storage_test_time_barrier(t, "spawn");
    *used = t->b->column_space.used >> 10;
    return (0);
}

int storage_test_weigh_tags(storage_test_t *t)
{
    uint32_t used[2] = {};

    if (use_columns(t, FRAG4096_PATH, &used[0]))
        return (1);
    if (use_columns(t, FRAGTAG4096_PATH, &used[1]))
        return (1);
    printf("204800 entities with six markers each: columns in use %u KB as "
        "components of four bytes, %u KB as tags\n", used[0], used[1]);
    return (0);
}

int storage_test_weigh_single(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(SINGLE_PATH, 8, 1 << 20, 4096);
    uint64_t best;

    if (storage_test_reopen(t, storage_test_hear_counts, &a))
        return (1);
    (void)storage_test_time_barrier(t, "attach_a");
    best = storage_test_time_least(t, "one", STORAGE_TEST_WEIGH_ROUNDS * 4);
    if (t->said[0] != 10'000'000) {
        printf("FAIL: it read %u\n", t->said[0]);
        return (1);
    }
    printf("a singleton read where it is: %6.3f ns each time\n",
        (double)best / 1'000'000);
    return (0);
}

int storage_test_weigh_switch(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(TOGGLE2_PATH, 204'800, 8 << 20,
        128 << 20);
    uint64_t best;

    if (storage_test_reopen(t, storage_test_hear_counts, &a))
        return (1);
    (void)storage_test_time_barrier(t, "spawn");
    best = storage_test_time_least(t, "flicker", STORAGE_TEST_WEIGH_ROUNDS * 4);
    printf("a toggled row switched by a write, with the draw that says which "
        "way: %6.2f ns a row\n", (double)best / 204'800);
    return (0);
}
