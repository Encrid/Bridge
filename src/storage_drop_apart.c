/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** src/storage_drop_apart.c -- a component that is not kept in a table,
** dropped: direct-indexed, kept in a sparse set, or a singleton.
*/

#include "storage_drop.h"
#include "storage_unload.h"

/*
** drop_cell
**
** Drops a direct-indexed component from the entity at `index`: the entity is
** destroyed if the policy says so of what it is left with, and otherwise
** loses its cell, with whoever observes it told.
*/
static void drop_cell(storage_t *r, const storage_type_t *type,
    enum bridge_on_unload policy, uint32_t index)
{
    uint64_t id = (uint64_t)r->places[index].generation << 32 | index;

    see_less(r, index, type, r->scratch);
    if (is_swept(r, policy, r->scratch)) {
        destroy_entity(r, index);
        return;
    }
    tell_observers(r, &(seen_t){ id, r->before, r->scratch, nullptr });
    give_cell(r, type->slot, index, false);
}

/*
** storage_drop_cells
**
** Drops a direct-indexed component: every entity that has been given one.
*/
void storage_drop_cells(storage_t *r, const storage_type_t *type,
    enum bridge_on_unload policy)
{
    uint32_t index = 1;

    while (index < r->high) {
        if (set_has(get_given(r, type->slot), index))
            drop_cell(r, type, policy, index);
        index++;
    }
    sweep_sets(r);
}

/*
** drop_member
**
** Drops a sparse set's component from the entity `id`, which is in it.
*/
static void drop_member(storage_t *r, const storage_type_t *type,
    enum bridge_on_unload policy, uint64_t id)
{
    uint32_t index = (uint32_t)id;

    see_less(r, index, type, r->scratch);
    if (is_swept(r, policy, r->scratch)) {
        destroy_entity(r, index);
        return;
    }
    tell_observers(r, &(seen_t){ id, r->before, r->scratch, nullptr });
    leave_set(r, type->slot, index);
}

/*
** storage_drop_set
**
** Drops a component kept in a sparse set: an entity at a time, from the
** last, until its table is gone or has no row.
*/
void storage_drop_set(storage_t *r, const storage_type_t *type,
    enum bridge_on_unload policy)
{
    uint32_t t = r->set_table[type->slot];
    const table_t *tab = &r->tables[t];

    while (t && tab->rows) {
        drop_member(r, type, policy, get_who(r, tab)[tab->rows - 1]);
        t = r->set_table[type->slot];
        tab = &r->tables[t];
    }
    sweep_sets(r);
}

/*
** storage_drop_single
**
** Drops a singleton: it has no value, and whoever observes it is told.
*/
void storage_drop_single(storage_t *r, storage_type_t *type)
{
    if (!type->set)
        return;
    see_less(r, 0, type, r->scratch);
    tell_observers(r, &(seen_t){ 0, r->before, r->scratch, nullptr });
    set_single(r, type, false);
}
