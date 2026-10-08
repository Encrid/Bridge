/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/storage_unload.h -- one entity at a time, outside any barrier: what
** an unload and a release share.
**
** lifetime.md, *What unload leaves behind*
*/

#ifndef BRIDGE_SRC_STORAGE_UNLOAD_H
    #define BRIDGE_SRC_STORAGE_UNLOAD_H

    #include "storage_carried.h"
    #include "storage_make.h"
    #include "storage_tell.h"

/*
** destroy_entity
**
** Destroys the entity at `index`, tells whoever observes what it carried,
** and gives back its table if it was the last in it.
*/
static inline void destroy_entity(storage_t *r, uint32_t index)
{
    uint64_t id = (uint64_t)r->places[index].generation << 32 | index;
    uint32_t from = r->places[index].table;

    get_carried(r, index, r->before);
    tell_observers(r, &(seen_t){ id, r->before, nullptr, nullptr });
    kill_entity(r, index);
    if (from && r->tables[from].live && !r->tables[from].rows)
        free_table(r, from);
}

/*
** sweep_sets
**
** Gives back the table of each sparse set nothing is left in, as a
** barrier's end does.
*/
static inline void sweep_sets(storage_t *r)
{
    uint32_t t;
    uint32_t slot = 0;

    while (slot < r->set_room) {
        t = r->set_table[slot];
        if (t && !r->tables[t].rows)
            free_table(r, t);
        slot++;
    }
}

/*
** is_apart
**
** Answers whether the entity at `index` carries something that is not kept
** in a table.
*/
static inline bool is_apart(const storage_t *r, uint32_t index)
{
    uint32_t slot = 0;

    while (slot < r->set_room) {
        if (r->set_type[slot] && get_where(r, slot)[index])
            return (true);
        slot++;
    }
    slot = 0;
    while (slot < r->direct_room) {
        if (r->direct_type[slot] && set_has(get_given(r, slot), index))
            return (true);
        slot++;
    }
    return (false);
}

/*
** see_less
**
** Writes into r->before what the entity at `index` carries, and into `left`
** the same without a type: what it will carry once the type is dropped.
*/
static inline void see_less(storage_t *r, uint32_t index,
    const storage_type_t *type, uint64_t *left)
{
    get_carried(r, index, r->before);
    set_copy(r, left, r->before);
    set_take(left, type->id);
}

/*
** is_swept
**
** Answers whether a policy destroys an entity that is left carrying `left`:
** `delete` always, and `sweep` when nothing is left.
*/
static inline bool is_swept(const storage_t *r, enum bridge_on_unload policy,
    const uint64_t *left)
{
    if (policy == BRIDGE_DELETE)
        return (true);
    return (policy == BRIDGE_SWEEP && set_is_empty(r, left));
}

#endif
