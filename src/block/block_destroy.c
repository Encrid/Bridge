/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "block.h"

static void release_shared(bridge_world_t *w)
{
    uint32_t i = 0;

    while (i < w->backend_count) {
        if (w->backends[i].shared)
            w->backends[i].api.memory_release(w->backends[i].shared);
        i++;
    }
    if (w->shared)
        os_release(w->shared, w->shared_reserve);
}

static void free_types(bridge_world_t *w)
{
    uint32_t i = 0;

    while (i < w->type_count) {
        module_free_type(w->types[i]);
        i++;
    }
    free(w->types);
}

void block_destroy(bridge_world_t *w)
{
    uint32_t i = 0;

    if (w->storage)
        storage_destroy(w->storage);
    release_shared(w);
    free(w->column_space.gaps);
    free(w->arena_space.gaps);
    free_types(w);
    while (i < w->kept_count) {
        free(w->kept[i].stem);
        free(w->kept[i].owned);
        i++;
    }
    free(w->kept);
}
