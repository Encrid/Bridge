/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/storage/storage_test_toggle.c -- a toggled component through every
** change of its table, the same with no room for its bits, and a tag in
** each storage layout.
**
** storage.md, *A toggle*
*/

#include <stdio.h>

#include "storage_test.h"

/*
** Six entities, each on when it is given. `flicker` turns the even ones
** off: a write, and nothing is heard. With attach_b three move to another
** table, on or off as they were. value_a is a value for one that is off:
** it has it, and stays off. With destroy_half the last row of each table
** takes the place, bit and all. respawn_half makes six more, on: the table
** moves to grow, bits and all.
*/
static const char *const TOGGLE_MOVED[] = {
    "spawn", "look", "flicker", "look", "look_b", "find", "attach_b", "look",
    "find", "value_a", "look", "find", "destroy_half", "look", "find",
    "respawn_half", "look", "find", nullptr
};

/*
** After A is stripped, the column before it gone, T and its bits have
** closed up. `combine` turns every one on.
*/
static const char *const TOGGLE_CLOSED_UP[] = {
    "look", "find", "combine", "look", nullptr
};

static const char *const TOGGLE_FULL[] = {
    "spawn", "look", "flicker", "look", nullptr
};

/*
** `flicker` turns the toggled tag off for three, attach_a gives the
** singleton, and detach_b takes the tag that is kept in tables: one table
** of them again.
*/
static const char *const TAG[] = {
    "spawn", "look", "flicker", "attach_a", "look", "detach_b", "look",
    nullptr
};

/*
** storage_test_toggle
**
** A toggled component through every change of its table, and then
** stripped.
*/
enum storage_test_end storage_test_toggle(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(NTOGGLE_PATH, 16, 1 << 20, 4096);

    if (storage_test_try_load(t, &a))
        return (STORAGE_TEST_LEFT);
    storage_test_run_all(t, TOGGLE_MOVED);
    storage_test_drop(t, "st:A", BRIDGE_STRIP);
    storage_test_run_all(t, TOGGLE_CLOSED_UP);
    storage_test_drop(t, "st:T", BRIDGE_STRIP);
    storage_test_run(t, "look");
    return (STORAGE_TEST_DONE);
}

/*
** storage_test_toggle_full
**
** A table with a toggled column and no room for all its rows: its bits and
** the bytes that say where they are need room too, and four of six fit.
**
** budgets.md, *What running out does*
*/
enum storage_test_end storage_test_toggle_full(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(NTOGGLE_PATH, 16, 1 << 20, 4096);

    a.cfg.max_system_failures = 4;
    if (storage_test_try_load(t, &a))
        return (STORAGE_TEST_LEFT);
    storage_test_run_all(t, TOGGLE_FULL);
    return (STORAGE_TEST_DONE);
}

/*
** storage_test_tag
**
** A tag in each storage layout, and what they take of [budget] columns.
**
** storage.md, *A tag*
*/
enum storage_test_end storage_test_tag(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(NTAG_PATH, 8, 1 << 20, 4096);

    if (storage_test_try_load(t, &a))
        return (STORAGE_TEST_LEFT);
    storage_test_run_all(t, TAG);
    printf("columns in use %u bytes\n", t->b->column_space.used);
    return (STORAGE_TEST_DONE);
}
