/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "block.h"

static void release_shared(bridge_t *b)
{
    uint32_t i = 0;

    while (i < b->backend_count) {
        if (b->backends[i].shared)
            b->backends[i].api.memory_release(b->backends[i].shared);
        i++;
    }
    if (b->shared)
        os_release(b->shared, b->shared_reserve);
}

static void free_types(bridge_t *b)
{
    uint32_t i = 0;

    while (i < b->type_count) {
        module_free_type(b->types[i]);
        i++;
    }
    free(b->types);
}

void block_destroy(bridge_t *b)
{
    uint32_t i = 0;

    if (b->storage)
        storage_destroy(b->storage);
    release_shared(b);
    free(b->column_space.gaps);
    free(b->arena_space.gaps);
    free_types(b);
    while (i < b->kept_count) {
        free(b->kept[i].stem);
        free(b->kept[i].owned);
        i++;
    }
    free(b->kept);
}
