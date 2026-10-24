/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "storage_test.h"

enum storage_test_end storage_test_fill_list(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(FULL_LIST_PATH, 2000, 256,
        1 << 20);

    if (storage_test_load(t, &a))
        return (STORAGE_TEST_FAILED);
    storage_test_run(t, "spawn");
    return (STORAGE_TEST_DONE);
}
