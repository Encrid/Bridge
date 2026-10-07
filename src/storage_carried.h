/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** src/storage_carried.h -- everything an entity carries: what its table
** holds, every sparse set it is in, and every direct-indexed component it
** has been given.
*/

#ifndef BRIDGE_SRC_STORAGE_CARRIED_H
    #define BRIDGE_SRC_STORAGE_CARRIED_H

    #include "storage_reach.h"
    #include "storage_set.h"
    #include "storage_sets.h"

/*
** add_sets
**
** Adds to a set the type of every sparse set the entity at `index` is in.
*/
static inline void add_sets(const storage_t *r, uint32_t index, uint64_t *set)
{
    uint32_t slot = 0;

    while (slot < r->set_room) {
        if (r->set_type[slot] && get_where(r, slot)[index])
            set_put(set, r->set_type[slot]->id);
        slot++;
    }
}

/*
** add_cells
**
** Adds to a set the type of every direct-indexed component the entity at
** `index` has been given.
*/
static inline void add_cells(const storage_t *r, uint32_t index, uint64_t *set)
{
    uint32_t slot = 0;

    while (slot < r->direct_room) {
        if (r->direct_type[slot] && set_has(get_given(r, slot), index))
            set_put(set, r->direct_type[slot]->id);
        slot++;
    }
}

/*
** add_singles
**
** Adds to a set every singleton that has a value: index 0, which is no
** entity, is what such a singleton is on.
*/
static inline void add_singles(const storage_t *r, uint64_t *set)
{
    uint32_t id = 0;

    while (id < r->type_room) {
        if (r->types[id] && r->types[id]->set)
            set_put(set, id);
        id++;
    }
}

/*
** get_carried
**
** Writes into a set everything the entity at `index` carries. While no type
** is kept apart from tables, that is what its table holds and no more.
*/
[[gnu::always_inline]] static inline void get_carried(const storage_t *r,
    uint32_t index, uint64_t *set)
{
    uint32_t t = r->places[index].table;

    if (t)
        set_copy(r, set, get_set(r, t));
    else
        set_clear(r, set);
    if (!r->apart)
        return;
    add_sets(r, index, set);
    add_cells(r, index, set);
    if (!index)
        add_singles(r, set);
}

#endif
