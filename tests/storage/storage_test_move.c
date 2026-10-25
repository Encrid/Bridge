/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "host_copy.h"
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

static void say_kept(bridge_world_t *w)
{
    const declared_type_t *c;
    uint32_t i = 0;

    while (i < w->type_count) {
        c = w->types[i];
        if (!strcmp(c->name, "st:X"))
            printf("  storage: st:X is %s, carried by %u\n",
                KEPT_AS[c->storage],
                storage_count_carriers(w->storage, c->type));
        i++;
    }
}

static const char *reload_copy(bridge_world_t *w, const bridge_artifact_t *a,
    void **spoiled)
{
    host_copy_t c;
    const bridge_artifact_t *copy = host_copy(&c, a, 1);
    const char *said = bridge_reload(w, copy, 1) ? "refused" : "done";

    host_spoil(spoiled, &c);
    return (said);
}

static void move(storage_test_t *t, const storage_test_moved_t *row)
{
    bridge_world_t *w = bridge_create(&t->cfg, storage_test_hear, t);
    bridge_artifact_t a = storage_test_state(BUILDS[row->from], 8, 1 << 20,
        4096);
    void *spoiled = nullptr;

    printf("%s to %s%s%s\n", KEPT_AS[row->from], KEPT_AS[row->to],
        *row->prepare ? ", after " : "", row->prepare);
    storage_test_load_reloadable(w, &a);
    (void)bridge_run_phase(w, "spawn");
    if (*row->prepare)
        (void)bridge_run_phase(w, row->prepare);
    (void)bridge_run_phase(w, "look");
    a.path = BUILDS[row->to];
    a.cfg.losses = row->losses;
    a.cfg.loss_count = row->loss_count;
    printf("  reload: %s\n", reload_copy(w, &a, &spoiled));
    (void)bridge_run_phase(w, "look");
    say_kept(w);
    (void)bridge_destroy(w);
    host_free_spoiled(&spoiled);
}

enum storage_test_end storage_test_move(storage_test_t *t)
{
    constexpr size_t count = sizeof(MOVED) / sizeof(*MOVED);
    size_t i = 0;

    (void)bridge_destroy(t->w);
    t->w = nullptr;
    t->cfg.budget.sparse_sets = 1;
    t->cfg.budget.direct_indexed = 1;
    while (i < count) {
        move(t, &MOVED[i]);
        i++;
    }
    t->cfg.budget.columns = 112;
    move(t, &MOVED_WITHOUT_ROOM);
    t->w = bridge_create(&t->cfg, storage_test_hear, t);
    return (STORAGE_TEST_DONE);
}
