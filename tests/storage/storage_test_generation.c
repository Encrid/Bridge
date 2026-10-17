/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "storage_inside.h"
#include "storage_test.h"

enum storage_test_end storage_test_wrap_generation(storage_test_t *t)
{
    storage_t *r = t->b->storage;
    uint64_t id;
    uint32_t index;

    if (storage_reserve(r, &id, 1) != 1)
        return (STORAGE_TEST_FAILED);
    index = (uint32_t)id;
    r->places[index].generation = UINT32_MAX - 1;
    id = (uint64_t)(UINT32_MAX - 1) << 32 | index;
    storage_release(r, &id, 1);
    (void)storage_reserve(r, &id, 1);
    printf("index %u, killed at generation %u, comes back at generation %u\n",
        index, UINT32_MAX - 1, (uint32_t)(id >> 32));
    return (STORAGE_TEST_DONE);
}
