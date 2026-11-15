/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "storage.h"
#include "storage_test.h"

static uint32_t count_values(storage_test_t *t, const char *name)
{
    uint32_t read = 0;

    storage_visit_values(t->w->storage, storage_test_find_type(t->w, name),
        storage_test_count_value, &read);
    return (read);
}

enum storage_test_end storage_test_call_on_tables(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(TOGGLE_PATH, 16, 1 << 20, 4096);
    uint32_t read[2] = {};
    uint32_t tables;

    if (storage_test_try_load(t, &a))
        return (STORAGE_TEST_DONE);
    storage_test_run(t, "spawn");
    storage_test_run(t, "attach_b");
    read[0] = count_values(t, "st:B");
    tables = storage_usage(t->w->storage).tables;
    storage_test_drop(t, "st:B", BRIDGE_DELETE);
    read[1] = count_values(t, "st:A");
    storage_validate_baseline(t->w->storage);
    printf("values of st:B read: %u in %u tables; values of st:A read: %u "
        "after the entities of st:B left, in %u tables\n", read[0], tables,
        read[1], storage_usage(t->w->storage).tables);
    return (STORAGE_TEST_DONE);
}

enum storage_test_end storage_test_call_on_direct(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(DIRECT_PATH, 8, 1 << 20, 4096);
    storage_type_t *large;

    if (storage_test_try_load(t, &a))
        return (STORAGE_TEST_DONE);
    storage_test_run(t, "spawn");
    storage_test_run(t, "attach_b");
    printf("values of st:D read by index: %u\n", count_values(t, "st:D"));
    large = storage_declare(t->w->storage, 8192, 8, BRIDGE_DIRECT_INDEXED);
    printf("a direct-indexed type of 8192 bytes for each entity: %s\n",
        large ? "declared" : "refused");
    return (STORAGE_TEST_DONE);
}
