/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/storage/storage_test_strip.c -- a component stripped from every
** entity, a table at a time: into the table of what is left, or where it
** stands, and with no room to merge.
**
** lifetime.md, *What unload leaves behind*
*/

#include "storage_test.h"

/*
** Three entities with A, and three with A and B. When B is stripped a
** table of what is left stands, with room: they join it. value_b makes the
** same two tables again. When A is stripped there is no table of B alone:
** the one of A and B becomes it, where it stands, and three entities are
** left with nothing. The column of B is where the index and the span say.
*/
static const char *const STRIP_TWO_TABLES[] = {
    "spawn", "attach_a", "value_a", "value_b", nullptr
};

static const char *const STRIP_JOINED[] = {
    "look", "look_b", "value_b", nullptr
};

static const char *const STRIP_BECOME[] = {
    "look", "look_b", "find", nullptr
};

/*
** B first, so that the table of A alone is made with room for its three
** rows and the region is then full. When B is stripped the table of A has
** no room for three more and cannot grow: two tables of A, and every
** entity still there. attach_a is then a value for each, which moves none
** of them, and both tables go when they empty.
*/
static const char *const FULL_BEFORE[] = {
    "spawn", "value_b", "attach_a", "look", nullptr
};

static const char *const FULL_AFTER[] = {
    "look", "find", "attach_a", "look", "destroy", "look", nullptr
};

/*
** storage_test_strip
**
** A component stripped where the table of what is left stands and has
** room, and where there is none.
*/
enum storage_test_end storage_test_strip(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(NSTORE_PATH, 6, 1 << 20, 1 << 20);

    if (storage_test_load(t, &a))
        return (STORAGE_TEST_FAILED);
    storage_test_run_all(t, STRIP_TWO_TABLES);
    storage_test_drop(t, "st:B", BRIDGE_STRIP);
    storage_test_run_all(t, STRIP_JOINED);
    storage_test_drop(t, "st:A", BRIDGE_STRIP);
    storage_test_run_all(t, STRIP_BECOME);
    return (STORAGE_TEST_DONE);
}

/*
** storage_test_strip_full
**
** A component stripped with no room to merge the rows into the table of
** what is left.
*/
enum storage_test_end storage_test_strip_full(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(NSTORE_PATH, 6, 1 << 20, 1 << 20);

    if (storage_test_load(t, &a))
        return (STORAGE_TEST_FAILED);
    storage_test_run_all(t, FULL_BEFORE);
    storage_test_drop(t, "st:B", BRIDGE_STRIP);
    storage_test_run_all(t, FULL_AFTER);
    return (STORAGE_TEST_DONE);
}
