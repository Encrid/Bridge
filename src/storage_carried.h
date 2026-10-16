/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_CARRIED_H
    #define BRIDGE_SRC_STORAGE_CARRIED_H

    #include "storage_reach.h"
    #include "storage_set.h"
    #include "storage_sets.h"

static inline void add_sets(const storage_t *r, uint32_t index, uint64_t *set)
{
    uint32_t slot = 0;

    while (slot < r->set_room) {
        if (r->set_type[slot] && get_where(r, slot)[index])
            set_put(set, r->set_type[slot]->id);
        slot++;
    }
}

static inline void add_cells(const storage_t *r, uint32_t index, uint64_t *set)
{
    uint32_t slot = 0;

    while (slot < r->direct_room) {
        if (r->direct_type[slot] && set_has(get_given(r, slot), index))
            set_put(set, r->direct_type[slot]->id);
        slot++;
    }
}

static inline void add_singles(const storage_t *r, uint64_t *set)
{
    uint32_t id = 0;

    while (id < r->type_room) {
        if (r->types[id] && r->types[id]->set)
            set_put(set, id);
        id++;
    }
}

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
