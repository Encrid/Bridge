/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "block.h"

static int map_block(bridge_world_t *w, const block_layout_t *layout)
{
    w->shared = os_reserve((size_t)layout->reserve, (size_t)layout->len);
    if (!w->shared) {
        say_error(w, "[budget] cannot be satisfied: the operating system "
            "would not give %llu bytes, in a reservation of %llu bytes",
            (unsigned long long)layout->len,
            (unsigned long long)layout->reserve);
        return (1);
    }
    w->shared_len = (size_t)layout->len;
    w->shared_reserve = (size_t)layout->reserve;
    w->index = (bridge_entity_t *)(w->shared + layout->index);
    w->sparse = (uint32_t)layout->sparse;
    w->sparse_sets = (uint32_t)layout->sets;
    w->direct = (uint32_t)layout->direct;
    w->direct_indexed = (uint32_t)layout->cells;
    w->directory = (uint32_t)layout->directory;
    w->arena = (uint32_t)layout->arena;
    w->columns = (uint32_t)layout->columns;
    return (0);
}

static void set_limits(bridge_world_t *w, const bridge_budget_t *budget)
{
    w->entities = (uint32_t)budget->entities;
    w->arena_len = (uint32_t)budget->persistent;
    w->columns_len = (uint32_t)budget->columns;
    w->max_modules = budget->modules;
    w->tables = budget->tables;
    w->max_types = budget->types;
    w->max_systems = budget->systems;
    w->max_terms = budget->terms;
}

static int make_storage(bridge_world_t *w)
{
    uint32_t room = w->tables + w->sparse_sets + w->max_types + 1;

    w->column_space = (region_t){ .room = room, .len = w->columns_len };
    w->column_space.gaps = malloc(room * sizeof(range_t));
    w->arena_space = (region_t){ .len = w->arena_len };
    if (bridge_lacks(w, w->column_space.gaps, "the list of what is free in "
        "[budget] columns"))
        return (1);
    w->storage = storage_create(w);
    return (!w->storage);
}

static int adopt_shared(bridge_world_t *w)
{
    backend_err_t err = {};
    open_backend_t *o = w->backends;
    uint32_t i = 0;

    while (i < w->backend_count) {
        o = &w->backends[i];
        o->shared = o->api.memory_adopt(o->be, w->shared, w->shared_len, &err);
        if (!o->shared) {
            say_error(w, "backend '%s' cannot adopt the shared memory: %s",
                o->name, err.text);
            return (1);
        }
        i++;
    }
    return (0);
}

int block_create(bridge_world_t *w, const bridge_budget_t *budget)
{
    block_layout_t layout = {};

    if (block_check_budget(w, budget) || block_lay_out(w, budget, &layout))
        return (1);
    if (map_block(w, &layout))
        return (1);
    set_limits(w, budget);
    if (make_storage(w))
        return (1);
    return (adopt_shared(w));
}
