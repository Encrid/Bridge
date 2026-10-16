/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "storage_test.h"

static const char *const TOGGLE_MOVED[] = {
    "spawn", "look", "flicker", "look", "look_b", "find", "attach_b", "look",
    "find", "value_a", "look", "find", "destroy_half", "look", "find",
    "respawn_half", "look", "find", nullptr
};

static const char *const TOGGLE_CLOSED_UP[] = {
    "look", "find", "combine", "look", nullptr
};

static const char *const TOGGLE_FULL[] = {
    "spawn", "look", "flicker", "look", nullptr
};

static const char *const TAG[] = {
    "spawn", "look", "flicker", "attach_a", "look", "detach_b", "look",
    nullptr
};

enum storage_test_end storage_test_toggle(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(TOGGLE_PATH, 16, 1 << 20, 4096);

    if (storage_test_try_load(t, &a))
        return (STORAGE_TEST_LEFT);
    storage_test_run_all(t, TOGGLE_MOVED);
    storage_test_drop(t, "st:A", BRIDGE_STRIP);
    storage_test_run_all(t, TOGGLE_CLOSED_UP);
    storage_test_drop(t, "st:T", BRIDGE_STRIP);
    storage_test_run(t, "look");
    return (STORAGE_TEST_DONE);
}

enum storage_test_end storage_test_toggle_full(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(TOGGLE_PATH, 16, 1 << 20, 4096);

    a.cfg.max_system_failures = 4;
    if (storage_test_try_load(t, &a))
        return (STORAGE_TEST_LEFT);
    storage_test_run_all(t, TOGGLE_FULL);
    return (STORAGE_TEST_DONE);
}

enum storage_test_end storage_test_tag(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(TAG_PATH, 8, 1 << 20, 4096);

    if (storage_test_try_load(t, &a))
        return (STORAGE_TEST_LEFT);
    storage_test_run_all(t, TAG);
    printf("columns in use %u bytes\n", t->b->column_space.used);
    return (STORAGE_TEST_DONE);
}
