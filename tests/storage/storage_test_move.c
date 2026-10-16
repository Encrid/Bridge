/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "storage_test.h"

static const char *const KEPT_AS[] = {
    "", "a table", "a sparse set", "a toggle", "direct-indexed", "a singleton"
};

static const char *const BUILDS[] = {
    "", MOVE1_PATH, MOVE2_PATH, MOVE3_PATH, MOVE4_PATH, MOVE5_PATH
};

static const char *const X_LOST[1] = { "st:X" };

static const storage_test_moved_t MOVED[] = {
    { 1, 2, "", nullptr, BRIDGE_NONE }, { 1, 3, "", nullptr, BRIDGE_NONE },
    { 1, 4, "", nullptr, BRIDGE_NONE }, { 2, 1, "", nullptr, BRIDGE_NONE },
    { 2, 3, "", nullptr, BRIDGE_NONE }, { 2, 4, "", nullptr, BRIDGE_NONE },
    { 3, 1, "", nullptr, BRIDGE_NONE }, { 3, 2, "", nullptr, BRIDGE_NONE },
    { 3, 4, "", nullptr, BRIDGE_NONE }, { 4, 1, "", nullptr, BRIDGE_NONE },
    { 4, 2, "", nullptr, BRIDGE_NONE }, { 4, 3, "", nullptr, BRIDGE_NONE },
    { 1, 5, "detach_b", nullptr, BRIDGE_NONE },
    { 2, 5, "detach_b", nullptr, BRIDGE_NONE },
    { 3, 5, "detach_b_half", nullptr, BRIDGE_NONE },
    { 4, 5, "", nullptr, BRIDGE_NONE },
    { 5, 1, "", nullptr, BRIDGE_NONE },
    { 3, 1, "flicker", nullptr, BRIDGE_NONE },
    { 3, 1, "flicker", X_LOST, 1 },
    { 1, 2, "", X_LOST, 1 }
};

static const storage_test_moved_t MOVED_WITHOUT_ROOM = { 1, 2, "", nullptr,
    BRIDGE_NONE };

static void say_kept(bridge_t *b)
{
    const declared_type_t *c;
    uint32_t i = 0;

    while (i < b->type_count) {
        c = b->types[i];
        if (!strcmp(c->name, "st:X"))
            printf("  storage: st:X is %s, carried by %u\n",
                KEPT_AS[c->storage],
                storage_count_carriers(b->storage, c->type));
        i++;
    }
}

static void move(storage_test_t *t, const storage_test_moved_t *row)
{
    bridge_t *b = bridge_create(&t->cfg, storage_test_hear, t);
    bridge_artifact_t a = storage_test_state(BUILDS[row->from], 8, 1 << 20,
        4096);

    printf("%s to %s%s%s\n", KEPT_AS[row->from], KEPT_AS[row->to],
        *row->prepare ? ", after " : "", row->prepare);
    storage_test_load_reloadable(b, &a);
    (void)bridge_run_phase(b, "spawn");
    if (*row->prepare)
        (void)bridge_run_phase(b, row->prepare);
    (void)bridge_run_phase(b, "look");
    a.path = BUILDS[row->to];
    a.cfg.losses = row->losses;
    a.cfg.loss_count = row->loss_count;
    printf("  reload: %s\n", bridge_reload(b, &a) ? "refused" : "done");
    (void)bridge_run_phase(b, "look");
    say_kept(b);
    (void)bridge_destroy(b);
}

enum storage_test_end storage_test_move(storage_test_t *t)
{
    constexpr size_t count = sizeof(MOVED) / sizeof(*MOVED);
    size_t i = 0;

    (void)bridge_destroy(t->b);
    t->cfg.budget.sparse_sets = 1;
    t->cfg.budget.direct_indexed = 1;
    while (i < count) {
        move(t, &MOVED[i]);
        i++;
    }
    t->cfg.budget.columns = 112;
    move(t, &MOVED_WITHOUT_ROOM);
    t->b = bridge_create(&t->cfg, storage_test_hear, t);
    return (STORAGE_TEST_DONE);
}
