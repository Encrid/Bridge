/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_PLACE_H
    #define BRIDGE_SRC_STORAGE_PLACE_H

    #include "storage_fold.h"
    #include "storage_tell.h"

static inline void place_sets(storage_t *r, uint32_t index)
{
    const uint64_t *work = get_work(r, index);
    uint32_t slot = 0;

    while (slot < r->set_room) {
        if (is_joining(r, slot, index))
            join_set(r, slot, index);
        else if (r->set_type[slot] && get_where(r, slot)[index]
            && !set_has(work, r->set_type[slot]->id))
            leave_set(r, slot, index);
        slot++;
    }
}

static inline void place_cells(storage_t *r, uint32_t index)
{
    const uint64_t *work = get_work(r, index);
    uint32_t slot = 0;
    bool is;

    while (slot < r->direct_room) {
        is = r->direct_type[slot] && set_has(work, r->direct_type[slot]->id);
        if (r->direct_type[slot] && set_has(get_given(r, slot), index) != is)
            give_cell(r, slot, index, is);
        slot++;
    }
}

static inline void place_singles(storage_t *r)
{
    storage_type_t *type;
    uint32_t id = 0;

    while (id < r->type_room) {
        type = r->types[id];
        if (type && type->kept == BRIDGE_SINGLETON
            && type->set != set_has(r->work, id))
            set_single(r, type, !type->set);
        id++;
    }
}

[[gnu::always_inline]] static inline void place_one(storage_t *r,
    uint32_t index)
{
    if (r->folded[index] & DESTROYED) {
        kill_entity(r, index);
        return;
    }
    move_row(r, index, r->dest[index]);
    if (!r->apart)
        return;
    place_sets(r, index);
    place_cells(r, index);
    if (!index)
        place_singles(r);
}

#endif
