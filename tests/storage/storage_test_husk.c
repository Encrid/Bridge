/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "storage_test.h"

static const char *const HUSK_MADE[] = {
    "spawn", "attach_both", "look", "look_b", nullptr
};

static const char *const HUSK_SEEN[] = {
    "look", "look_b", nullptr
};

enum storage_test_end storage_test_delete_husk(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(STOREHUSK_PATH, 6, 1 << 20,
        1 << 20);

    if (storage_test_load(t, &a))
        return (STORAGE_TEST_FAILED);
    storage_test_run_all(t, HUSK_MADE);
    storage_test_drop(t, "st:A", BRIDGE_DELETE);
    storage_test_say_alive(t, 6);
    storage_test_run_all(t, HUSK_SEEN);
    storage_test_drop(t, "st:B", BRIDGE_STRIP);
    storage_test_say_alive(t, 6);
    storage_test_run(t, "look_b");
    return (STORAGE_TEST_DONE);
}
