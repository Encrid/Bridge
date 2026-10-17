/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "storage_test.h"

static const char *const CHANGES[] = {
    "spawn", "attach_a", "look", "value_a", "look", "attach_b_half", "look",
    "attach_a", "detach_b_absent", "flicker", "look", "detach_b_half",
    "look", "destroy_half", "look", "respawn_half", "touch_dead", "look",
    "destroy_half", "respawn_half", "destroy_half", "respawn_half",
    "destroy_half", "respawn_half", "recycled", "attach_a_half", "attach_b",
    "look", "destroy", "look", nullptr
};

static const char *const TABLES[] = {
    "spawn", "combine", "look", "attach_a", "look", "destroy", "look",
    nullptr
};

static const char *const NO_TABLES[] = {
    "spawn", "attach_a", "attach_b_half", "look", "attach_b_half",
    "attach_b_half", "look", nullptr
};

static const char *const NO_TABLES_STALE[] = {
    "spawn", "attach_a", "destroy_half", "respawn_half", "stale", "look",
    nullptr
};

static const char *const NO_COLUMNS[] = {
    "spawn", "attach_a", "look", "attach_a", "look", "attach_a", "look",
    "attach_a", "attach_a", nullptr
};

static const char *const SCRIBBLE[] = {
    "spawn", "attach_a", "scribble", "destroy_half", "look", "attach_b_half",
    "look", "find", nullptr
};

enum storage_test_end storage_test_change(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(STORE_PATH, 6, 1 << 20, 1 << 20);

    if (storage_test_load(t, &a))
        return (STORAGE_TEST_FAILED);
    storage_test_run_all(t, CHANGES);
    return (STORAGE_TEST_DONE);
}

enum storage_test_end storage_test_make_tables(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(STORETAB_PATH, 10'000, 8 << 20,
        32 << 20);

    if (storage_test_load(t, &a))
        return (STORAGE_TEST_FAILED);
    storage_test_run_all(t, TABLES);
    return (STORAGE_TEST_DONE);
}

enum storage_test_end storage_test_use_up_tables(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(STORE_PATH, 6, 1 << 20, 1 << 20);

    a.cfg.max_system_failures = 2;
    if (storage_test_load(t, &a))
        return (STORAGE_TEST_FAILED);
    storage_test_run_all(t, strcmp(t->what, "notables") ? NO_TABLES_STALE
        : NO_TABLES);
    return (STORAGE_TEST_DONE);
}

enum storage_test_end storage_test_use_up_columns(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(STORE_PATH, 6, 1 << 20, 1 << 20);

    a.cfg.max_system_failures = 4;
    if (storage_test_load(t, &a))
        return (STORAGE_TEST_FAILED);
    storage_test_run_all(t, NO_COLUMNS);
    return (STORAGE_TEST_DONE);
}

enum storage_test_end storage_test_scribble(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(STORE_PATH, 6, 1 << 20, 1 << 20);

    if (storage_test_load(t, &a))
        return (STORAGE_TEST_FAILED);
    storage_test_run_all(t, SCRIBBLE);
    return (STORAGE_TEST_DONE);
}
