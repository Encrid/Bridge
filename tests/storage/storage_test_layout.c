/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "storage_test.h"

static const char *const B_GONE[1] = { "st:X.b" };
static const char *const ARR_CUT[1] = { "st:X.arr" };
static const char *const Y_GONE[1] = { "st:X.at.y" };
static const char *const ALL_GONE[5] = { "st:X.a", "st:X.b", "st:X.arr",
    "st:X.at", "st:X.mode" };
static const char *const ARR_AND_B[2] = { "st:X.arr", "st:X.b" };

static const storage_test_relaid_t RELAID[] = {
    { LAYOUT0_PATH, LAYOUT1_PATH, "a field added", nullptr, BRIDGE_NONE },
    { LAYOUT0_PATH, LAYOUT2_PATH, "the fields in another order", nullptr,
        BRIDGE_NONE },
    { LAYOUT0_PATH, LAYOUT3_PATH, "an array made longer", nullptr,
        BRIDGE_NONE },
    { LAYOUT0_PATH, LAYOUT4_PATH, "a field removed", nullptr, BRIDGE_NONE },
    { LAYOUT0_PATH, LAYOUT4_PATH, "a field removed, and that accepted",
        B_GONE, 1 },
    { LAYOUT0_PATH, LAYOUT5_PATH, "an array made shorter", nullptr,
        BRIDGE_NONE },
    { LAYOUT0_PATH, LAYOUT5_PATH,
        "an array made shorter, and that accepted", ARR_CUT, 1 },
    { LAYOUT0_PATH, LAYOUT6_PATH, "a field renamed", nullptr, BRIDGE_NONE },
    { LAYOUT0_PATH, LAYOUT6_PATH,
        "a field renamed, and the old one's loss accepted", B_GONE, 1 },
    { LAYOUT0_PATH, LAYOUT7_PATH, "a field of another type", nullptr,
        BRIDGE_NONE },
    { LAYOUT0_PATH, LAYOUT8_PATH, "a struct a field names, changed",
        nullptr, BRIDGE_NONE },
    { LAYOUT0_PATH, LAYOUT11_PATH,
        "a struct a field names, with a field removed", nullptr, BRIDGE_NONE },
    { LAYOUT0_PATH, LAYOUT11_PATH,
        "a struct a field names, with a field removed, and that accepted",
        Y_GONE, 1 },
    { LAYOUT0_PATH, LAYOUT9_PATH,
        "an enum whose constants have other numbers", nullptr, BRIDGE_NONE },
    { LAYOUT0_PATH, LAYOUT10_PATH, "an enum with a constant added", nullptr,
        BRIDGE_NONE },
    { LAYOUT0_PATH, LAYOUT12_PATH, "an enum with a constant removed",
        nullptr, BRIDGE_NONE },
    { LAYOUT0_PATH, LAYOUT13_PATH, "a struct declared as a tag", nullptr,
        BRIDGE_NONE },
    { LAYOUT0_PATH, LAYOUT14_PATH, "an enum of a wider integer", nullptr,
        BRIDGE_NONE },
    { LAYOUT0_PATH, LAYOUT15_PATH,
        "every field renamed, and each loss accepted", ALL_GONE, 5 },
    { LAYOUT0_PATH, LAYOUT16_PATH, "a field made a struct of another type",
        nullptr, BRIDGE_NONE },
    { LAYOUT0_PATH, LAYOUT17_PATH,
        "a struct a field names, changed, in an array made longer", nullptr,
        BRIDGE_NONE },
    { LAYOUT18_PATH, LAYOUT0_PATH, "five fields of long names removed",
        nullptr, BRIDGE_NONE },
    { LAYOUT0_PATH, LAYOUT20_PATH, "a field added, declared twice", nullptr,
        BRIDGE_NONE },
    { LAYOUT0_PATH, LAYOUT21_PATH,
        "a field added, then another layout declared", nullptr, BRIDGE_NONE },
    { LAYOUT0_PATH, LAYOUT4_PATH,
        "a field removed, and its loss accepted with one that does not happen", ARR_AND_B, 2 }
};

static const storage_test_relaid_t RELAID_ELSEWHERE[] = {
    { LAYOUT0K2_PATH, LAYOUT1K2_PATH, "a field added, in a sparse set",
        nullptr, BRIDGE_NONE },
    { LAYOUT0K3_PATH, LAYOUT1K3_PATH, "a field added, in a toggle", nullptr,
        BRIDGE_NONE },
    { LAYOUT0K4_PATH, LAYOUT1K4_PATH, "a field added, direct-indexed",
        nullptr, BRIDGE_NONE },
    { LAYOUT0K5_PATH, LAYOUT1K5_PATH, "a field added, in a singleton",
        nullptr, BRIDGE_NONE },
    { LAYOUT0K2_PATH, LAYOUT2K2_PATH,
        "the fields in another order, in a sparse set", nullptr, BRIDGE_NONE },
    { LAYOUT0K3_PATH, LAYOUT2K3_PATH,
        "the fields in another order, in a toggle", nullptr, BRIDGE_NONE },
    { LAYOUT0K4_PATH, LAYOUT2K4_PATH,
        "the fields in another order, direct-indexed", nullptr, BRIDGE_NONE },
    { LAYOUT0K5_PATH, LAYOUT2K5_PATH,
        "the fields in another order, in a singleton", nullptr, BRIDGE_NONE },
    { LAYOUT0_PATH, LAYOUT1K3_PATH, "a field added, and the table made a toggle",
        nullptr, BRIDGE_NONE },
    { LAYOUT0K5_PATH, LAYOUT1_PATH,
        "a field added, and the singleton made a table", nullptr, BRIDGE_NONE }
};

static void relay(storage_test_t *t, const storage_test_relaid_t *row)
{
    bridge_t *b = bridge_create(&t->cfg, storage_test_hear, t);
    bridge_artifact_t a = storage_test_state(row->from, 8, 1 << 20, 4096);

    printf("%s\n", row->what);
    storage_test_load_reloadable(b, &a);
    (void)bridge_run_phase(b, "spawn");
    (void)bridge_run_phase(b, "look");
    a.path = row->to;
    a.cfg.losses = row->losses;
    a.cfg.loss_count = row->loss_count;
    printf("  reload: %s\n", bridge_reload(b, &a) ? "refused" : "done");
    (void)bridge_run_phase(b, "look");
    (void)bridge_destroy(b);
}

static void relay_all(storage_test_t *t, const storage_test_relaid_t *rows,
    size_t count)
{
    size_t i = 0;

    while (i < count) {
        relay(t, &rows[i]);
        i++;
    }
}

static int relay_held(storage_test_t *t)
{
    bridge_artifact_t holder = storage_test_state(HOLDER_PATH, BRIDGE_NONE,
        1 << 20, BRIDGE_NONE);
    bridge_t *b = bridge_create(&t->cfg, storage_test_hear, t);
    bridge_artifact_t a = storage_test_state(LAYOUT0_PATH, 8, 1 << 20, 4096);

    a.cfg.reloadable = BRIDGE_TRUE;
    a.cfg.hot_reload = BRIDGE_TRUE;
    holder.cfg.identity = "st:holder@0.1.0";
    printf("a struct a field of another module's component names\n");
    if (!b || bridge_load(b, &a) || bridge_load(b, &holder)) {
        printf("FAIL: not loaded\n");
        return (1);
    }
    a.path = LAYOUT8_PATH;
    printf("  reload: %s\n", bridge_reload(b, &a) ? "refused" : "done");
    (void)bridge_destroy(b);
    return (0);
}

enum storage_test_end storage_test_relay(storage_test_t *t)
{
    (void)bridge_destroy(t->b);
    t->cfg.budget.sparse_sets = 2;
    t->cfg.budget.direct_indexed = 2;
    relay_all(t, RELAID, sizeof(RELAID) / sizeof(*RELAID));
    if (relay_held(t))
        return (STORAGE_TEST_FAILED);
    relay_all(t, RELAID_ELSEWHERE,
        sizeof(RELAID_ELSEWHERE) / sizeof(*RELAID_ELSEWHERE));
    t->b = bridge_create(&t->cfg, storage_test_hear, t);
    return (STORAGE_TEST_DONE);
}
