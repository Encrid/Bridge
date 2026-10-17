/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "storage_test.h"

static const char *const STRIP_TWO_TABLES[] = {
    "spawn", "attach_a", "value_a", "value_b", nullptr
};

static const char *const STRIP_JOINED[] = {
    "look", "look_b", "value_b", nullptr
};

static const char *const STRIP_BECOME[] = {
    "look", "look_b", "find", nullptr
};

static const char *const FULL_BEFORE[] = {
    "spawn", "value_b", "attach_a", "look", nullptr
};

static const char *const THREE_BEFORE[] = {
    "spawn", "attach_three", "look", nullptr
};

static const char *const FULL_AFTER[] = {
    "look", "find", "attach_a", "look", "destroy", "look", nullptr
};

enum storage_test_end storage_test_strip(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(STORE_PATH, 6, 1 << 20, 1 << 20);

    if (storage_test_load(t, &a))
        return (STORAGE_TEST_FAILED);
    storage_test_run_all(t, STRIP_TWO_TABLES);
    storage_test_drop(t, "st:B", BRIDGE_STRIP);
    storage_test_run_all(t, STRIP_JOINED);
    storage_test_drop(t, "st:A", BRIDGE_STRIP);
    storage_test_run_all(t, STRIP_BECOME);
    return (STORAGE_TEST_DONE);
}

enum storage_test_end storage_test_strip_full(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(STORE_PATH, 6, 1 << 20, 1 << 20);

    if (storage_test_load(t, &a))
        return (STORAGE_TEST_FAILED);
    storage_test_run_all(t, FULL_BEFORE);
    storage_test_drop(t, "st:B", BRIDGE_STRIP);
    if (!strcmp(t->what, "stripfulltwice")) {
        storage_test_drop(t, "st:A", BRIDGE_STRIP);
        storage_test_run(t, "look");
        return (STORAGE_TEST_DONE);
    }
    storage_test_run_all(t, FULL_AFTER);
    return (STORAGE_TEST_DONE);
}

enum storage_test_end storage_test_strip_unnamed(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(STORE_PATH, 6, 1 << 20, 1 << 20);

    if (storage_test_load(t, &a))
        return (STORAGE_TEST_FAILED);
    storage_test_run_all(t, THREE_BEFORE);
    storage_test_drop(t, "st:B", BRIDGE_STRIP);
    storage_test_drop(t, "st:A", BRIDGE_STRIP);
    storage_test_run(t, "look");
    return (STORAGE_TEST_DONE);
}
