/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "storage_make.h"
#include "storage_publish.h"
#include "storage_test.h"

enum storage_test_end storage_test_table_full([[maybe_unused]] storage_test_t *t)
{
    uint64_t set[1] = { 1 };
    uint32_t hash[1] = { 0 };
    storage_type_t *set_type[1] = { nullptr };
    bridge_world_t world = { .tables = 10 };
    storage_t r = { .w = &world, .words = 1, .hash = hash, .hash_room = 1,
        .set_type = set_type, .set_room = 1, .table_high = 5,
        .table_room = 4 };

    printf("table past the room: %u\n", make_table(&r, set));
    return (STORAGE_TEST_DONE);
}

enum storage_test_end storage_test_spans_full([[maybe_unused]] storage_test_t *t)
{
    uint8_t buf[256] = {};
    bridge_world_t world = {};
    storage_t r = { .w = &world };
    module_t m = { .priv = buf, .declared_count = 1 };
    const char *list = nullptr;

    printf("spans too small: %d\n", storage_publish(&r, &m, &list));
    return (STORAGE_TEST_DONE);
}
