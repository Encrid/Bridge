/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "storage_test.h"

constexpr uint32_t BASELINE_ROUNDS = 32;
constexpr uint32_t BASELINE_ASKED = 1'000'000;

static const char *const BASELINE_BUILDS[] = {
    WEIGH1_PATH, WEIGH2_PATH, WEIGH16_PATH
};

static const storage_test_op_t BASELINE_READS[] = {
    { "walk", "read in order" },
    { "near", "reached by id, in the order they lie" },
    { "far", "reached by id, in no order" }
};

static int say_reads(storage_test_t *t)
{
    const storage_test_op_t *w = BASELINE_READS;
    uint64_t best;
    size_t i = 0;

    while (i < sizeof(BASELINE_READS) / sizeof(*BASELINE_READS)) {
        best = storage_test_time_least(t, w[i].phase, BASELINE_ROUNDS);
        if (t->said[0] != STORAGE_TEST_ROWS) {
            printf("FAIL: it read %u\n", t->said[0]);
            return (1);
        }
        printf("%u values %-38s %7.1f us, %6.3f ns each\n", STORAGE_TEST_ROWS,
            w[i].what, (double)best / 1000.0,
            (double)best / STORAGE_TEST_ROWS);
        i++;
    }
    return (0);
}

static void say_flagged(storage_test_t *t)
{
    uint64_t best = storage_test_time_least(t, "flag", BASELINE_ROUNDS);

    printf("%u rows tested by a field, %6u of them on  %7.1f us, %6.3f ns a "
        "row\n", STORAGE_TEST_ROWS, t->said[0], (double)best / 1000.0,
        (double)best / STORAGE_TEST_ROWS);
}

static int say_one(storage_test_t *t)
{
    uint64_t best = storage_test_time_least(t, "one", BASELINE_ROUNDS);

    if (t->said[0] != BASELINE_ASKED) {
        printf("FAIL: it read %u\n", t->said[0]);
        return (1);
    }
    printf("one value on one entity, found through its query  %6.3f ns each "
        "time\n", (double)best / BASELINE_ASKED);
    return (0);
}

static int weigh_build(storage_test_t *t, const char *path, bool first)
{
    bridge_artifact_t a = storage_test_state(path, STORAGE_TEST_ROWS + 1,
        8 << 20, 128 << 20);

    if (storage_test_reopen(t, storage_test_hear_counts, &a))
        return (1);
    (void)storage_test_time_barrier(t, "spawn");
    if (first && say_reads(t))
        return (1);
    say_flagged(t);
    if (first && say_one(t))
        return (1);
    return (0);
}

enum storage_test_end storage_test_take_baseline(storage_test_t *t)
{
    constexpr size_t count = sizeof(BASELINE_BUILDS) / sizeof(char *);
    size_t k = 0;

    t->cfg.budget.columns = 256 << 20;
    while (k < count) {
        if (weigh_build(t, BASELINE_BUILDS[k], !k))
            return (STORAGE_TEST_FAILED);
        k++;
    }
    return (STORAGE_TEST_DONE);
}
