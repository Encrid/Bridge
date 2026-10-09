/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/storage/storage_test_changes.c -- the changes a module can ask
** for, in tables: one at a time, ten thousand tables, and the budgets of
** tables and of columns used up.
*/

#include "storage_test.h"

/*
** Six entities gain A, and one of them is given a value. Three gain B: a
** second table. A again, on all six, is a set and no move. Then a detach
** of a component they do not have, and a flicker: there and gone, gone and
** back, inside one barrier. The second table is emptied. Three are
** destroyed, each twice, and three made in their place; changes are asked
** of an id that is gone. Ids are set aside a barrier ahead, so a freed
** index comes back some rounds later: three rounds, and then `recycled`.
** At the end all six carry both, and all six go with both on them.
*/
static const char *const CHANGES[] = {
    "spawn", "attach_a", "look", "value_a", "look", "attach_b_half", "look",
    "attach_a", "detach_b_absent", "flicker", "look", "detach_b_half",
    "look", "destroy_half", "look", "respawn_half", "touch_dead", "look",
    "destroy_half", "respawn_half", "destroy_half", "respawn_half",
    "destroy_half", "respawn_half", "recycled", "attach_a_half", "attach_b",
    "look", "destroy", "look", nullptr
};

/*
** `combine` leaves ten thousand entities, no two with the same set. With
** attach_a every one moves to a table that did not exist.
*/
static const char *const TABLES[] = {
    "spawn", "combine", "look", "attach_a", "look", "destroy", "look",
    nullptr
};

/*
** attach_a makes the one table there may be. attach_b_half asks for a
** second: three entities stay as they were, and are not heard. A barrier
** the module changed nothing in forgives nothing, so the second
** attach_b_half is its budget of two failures, and at the third it is
** stopped.
*/
static const char *const NO_TABLES[] = {
    "spawn", "attach_a", "attach_b_half", "look", "attach_b_half",
    "attach_b_half", "look", nullptr
};

/*
** Sixty-four bytes hold four rows of A and not six: three fit by halves,
** and four by going back up, and the module runs on. The second attach_a
** is a value for those four, and the other two are left again. They are
** never to be had: the fourth barrier that left them out is the module's
** budget of four failures, and at the fifth attach_a it is stopped.
*/
static const char *const NO_COLUMNS[] = {
    "spawn", "attach_a", "look", "attach_a", "look", "attach_a", "look",
    "attach_a", "attach_a", nullptr
};

/*
** After `scribble` every copy in the block names the first entity. With
** destroy_half rows leave, and the last is moved into each one's place.
** With attach_b_half the three that are left move to another table, and
** `find` says each is where the index says, under its own id.
*/
static const char *const SCRIBBLE[] = {
    "spawn", "attach_a", "scribble", "destroy_half", "look", "attach_b_half",
    "look", "find", nullptr
};

/*
** storage_test_change
**
** Every structural change a module can ask for, one at a time, and what a
** system and an observer see after each.
*/
enum storage_test_end storage_test_change(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(STORE_PATH, 6, 1 << 20, 1 << 20);

    if (storage_test_load(t, &a))
        return (STORAGE_TEST_FAILED);
    storage_test_run_all(t, CHANGES);
    return (STORAGE_TEST_DONE);
}

/*
** storage_test_make_tables
**
** Ten thousand tables, and ten thousand more.
*/
enum storage_test_end storage_test_make_tables(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(STORETAB_PATH, 10'000, 8 << 20,
        32 << 20);

    if (storage_test_load(t, &a))
        return (STORAGE_TEST_FAILED);
    storage_test_run_all(t, TABLES);
    return (STORAGE_TEST_DONE);
}

/*
** storage_test_use_up_tables
**
** [budget] tables used up: the entity is left as it was, and it is a
** failure of the module that asked, which has a budget of two.
*/
enum storage_test_end storage_test_use_up_tables(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(STORE_PATH, 6, 1 << 20, 1 << 20);

    a.cfg.max_system_failures = 2;
    if (storage_test_load(t, &a))
        return (STORAGE_TEST_FAILED);
    storage_test_run_all(t, NO_TABLES);
    return (STORAGE_TEST_DONE);
}

/*
** storage_test_use_up_columns
**
** [budget] columns used up: the same, of a module with a budget of four.
*/
enum storage_test_end storage_test_use_up_columns(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(STORE_PATH, 6, 1 << 20, 1 << 20);

    a.cfg.max_system_failures = 4;
    if (storage_test_load(t, &a))
        return (STORAGE_TEST_FAILED);
    storage_test_run_all(t, NO_COLUMNS);
    return (STORAGE_TEST_DONE);
}

/*
** storage_test_scribble
**
** A module writes over what storage publishes in the shared block, and
** storage goes by what it keeps itself.
*/
enum storage_test_end storage_test_scribble(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(STORE_PATH, 6, 1 << 20, 1 << 20);

    if (storage_test_load(t, &a))
        return (STORAGE_TEST_FAILED);
    storage_test_run_all(t, SCRIBBLE);
    return (STORAGE_TEST_DONE);
}
