/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/block_create.c -- the shared block made, and unmade.
*/

#include <stdlib.h>

#include "block.h"

/*
** map_block
**
** Reserves the shared block's address space and commits the block, and
** keeps where each of its parts begins.
**
** failure.md, *Out of memory*
*/
static int map_block(bridge_t *b, const block_layout_t *layout)
{
    b->shared = os_reserve((size_t)layout->reserve, (size_t)layout->len);
    if (!b->shared) {
        say_error(b, "[budget] cannot be satisfied: the operating system "
            "would not give %llu bytes", (unsigned long long)layout->len);
        return (1);
    }
    b->shared_len = (size_t)layout->len;
    b->shared_reserve = (size_t)layout->reserve;
    b->index = (bridge_entity_t *)(b->shared + layout->index);
    b->sparse = (uint32_t)layout->sparse;
    b->sparse_sets = (uint32_t)layout->sets;
    b->direct = (uint32_t)layout->direct;
    b->direct_indexed = (uint32_t)layout->cells;
    b->arena = (uint32_t)layout->arena;
    b->columns = (uint32_t)layout->columns;
    return (0);
}

static void set_limits(bridge_t *b, const bridge_budget_t *budget)
{
    b->entities = (uint32_t)budget->entities;
    b->arena_len = (uint32_t)budget->persistent;
    b->columns_len = (uint32_t)budget->columns;
    b->max_modules = budget->modules;
    b->tables = budget->tables;
    b->max_types = budget->types;
    b->max_systems = budget->systems;
    b->max_terms = budget->terms;
}

/*
** make_storage
**
** Makes the two regions' lists of what is free, and storage. A gap lies
** between two blocks. A table has one block, and so has a sparse set, and
** so has each type that is direct-indexed or a singleton, of which there
** are no more than types: never more gaps than those, and one.
*/
static int make_storage(bridge_t *b)
{
    uint32_t room = b->tables + b->sparse_sets + b->max_types + 1;

    b->column_space = (region_t){ .room = room, .len = b->columns_len };
    b->column_space.gaps = malloc(room * sizeof(range_t));
    b->arena_space = (region_t){ .len = b->arena_len };
    if (bridge_lacks(b, b->column_space.gaps, "the list of what is free in "
        "[budget] columns"))
        return (1);
    b->storage = storage_create(b);
    return (!b->storage);
}

/*
** adopt_shared
**
** Has every backend adopt the shared block, so that each can hand it to the
** modules it runs.
**
** backend.md, *Memory is adopted, not created*
*/
static int adopt_shared(bridge_t *b)
{
    backend_err_t err = {};
    open_backend_t *o = b->backends;
    uint32_t i = 0;

    while (i < b->backend_count) {
        o = &b->backends[i];
        o->shared = o->api.memory_adopt(o->be, b->shared, b->shared_len, &err);
        if (!o->shared) {
            say_error(b, "backend '%s' cannot adopt the shared memory: %s",
                o->name, err.text);
            return (1);
        }
        i++;
    }
    return (0);
}

/*
** block_create
**
** Makes the shared block a deployment's budget states, with storage in it,
** and has every backend adopt it.
*/
int block_create(bridge_t *b, const bridge_budget_t *budget)
{
    block_layout_t layout = {};

    if (block_check_budget(b, budget) || block_lay_out(b, budget, &layout))
        return (1);
    if (map_block(b, &layout))
        return (1);
    set_limits(b, budget);
    if (make_storage(b))
        return (1);
    return (adopt_shared(b));
}
