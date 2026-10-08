/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/storage/storage_test_move.c -- a module that keeps st:X in one
** storage layout, reloaded as a build that keeps it in another. What
** storage has of it afterwards is said from in here, whatever became of
** the module.
**
** lifetime.md, *When a component is moved*
*/

#include <stdio.h>
#include <string.h>

#include "storage_test.h"

/*
** What a transcript calls each storage layout, and the build that keeps
** st:X in it, by the layout's number.
*/
static const char *const KEPT_AS[] = {
    "", "a table", "a sparse set", "a toggle", "direct-indexed", "a singleton"
};

static const char *const BUILDS[] = {
    "", NMOVE1_PATH, NMOVE2_PATH, NMOVE3_PATH, NMOVE4_PATH, NMOVE5_PATH
};

static const char *const X_LOST[1] = { "st:X" };

/*
** Every one of the four layouts that an entity carries, to every other.
** Then to a singleton: from one entity, from none, and from three, which
** nothing chooses among. And from a singleton, which no entity is there to
** carry. A toggle with a row that is off is refused, naming the loss,
** until the loss is accepted. The last row is a loss that is accepted and
** not made.
*/
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

/*
** From a table to a sparse set with [budget] columns of 112 bytes: no room
** for the rows where they are to go, and nothing has moved.
*/
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

/*
** move
**
** One row, in a deck of its own: the first build loaded, run, prepared
** and looked at, the reload into the second, another look, and what
** storage has of st:X.
*/
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

/*
** storage_test_move
**
** Every move of a component between storage layouts, each in a deck of
** its own with room for one sparse set and one direct-indexed component.
** The deck main made is replaced by one of the last deployment.
*/
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
