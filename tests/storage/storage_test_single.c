/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "storage_test.h"

static const char *const SINGLE_GIVEN[] = {
    "look", "attach_a", "look", nullptr
};

static const char *const SINGLE_KEPT[] = {
    "value_a", "look", "work", "look", "detach_b", "look", "flicker", "look",
    "attach_a", "look", nullptr
};

static int give_once(storage_test_t *t, const char *path)
{
    bridge_artifact_t a = storage_test_state(path, 8, 1 << 20, 4096);

    if (storage_test_try_load(t, &a))
        return (1);
    storage_test_run_all(t, SINGLE_GIVEN);
    return (0);
}

enum storage_test_end storage_test_give_single(storage_test_t *t)
{
    if (give_once(t, SINGLE_PATH))
        return (STORAGE_TEST_DONE);
    storage_test_run_all(t, SINGLE_KEPT);
    storage_test_drop(t, "st:G", BRIDGE_DELETE);
    storage_test_run(t, "look");
    return (STORAGE_TEST_DONE);
}

enum storage_test_end storage_test_walk_single(storage_test_t *t)
{
    (void)give_once(t, SINGLEBAD1_PATH);
    return (STORAGE_TEST_DONE);
}

enum storage_test_end storage_test_attach_single_to_entity(storage_test_t *t)
{
    (void)give_once(t, SINGLEBAD2_PATH);
    return (STORAGE_TEST_DONE);
}

enum storage_test_end storage_test_attach_to_no_entity(storage_test_t *t)
{
    (void)give_once(t, SINGLEBAD3_PATH);
    return (STORAGE_TEST_DONE);
}
