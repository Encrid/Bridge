/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_ROOM_H
    #define BRIDGE_SRC_STORAGE_ROOM_H

    #include "storage_grow.h"
    #include "storage_reserve.h"

static inline uint32_t get_retry(const storage_t *r, uint32_t rows)
{
    return ((uint32_t)((uint64_t)rows * r->w->table_retry / 100));
}

static inline void size_table(storage_t *r, uint32_t t)
{
    table_t *tab = &r->tables[t];
    uint32_t most = tab->incoming;
    uint32_t between;

    while (tab->incoming && !grow_table(r, t, tab->rows + tab->incoming))
        tab->incoming = get_retry(r, tab->incoming);
    while (most - tab->incoming > 1) {
        between = tab->incoming + (most - tab->incoming) / 2;
        if (grow_table(r, t, tab->rows + between))
            tab->incoming = between;
        else
            most = between;
    }
    tab->sized = true;
}

[[gnu::always_inline]] static inline bool make_room(storage_t *r, uint32_t t)
{
    table_t *tab = &r->tables[t];

    if (!tab->sized)
        size_table(r, t);
    if (!tab->incoming)
        return (false);
    tab->incoming--;
    return (true);
}

static inline bool room_joins(storage_t *r, uint32_t index)
{
    uint32_t slot = 0;

    while (slot < r->set_room) {
        if (is_joining(r, slot, index) && !make_room(r, r->set_table[slot]))
            return (false);
        slot++;
    }
    return (true);
}

[[gnu::always_inline]] static inline void room_one(storage_t *r,
    uint32_t index)
{
    uint32_t t = r->dest[index];

    if ((r->folded[index] & (DESTROYED | DROPPED | MOVES)) != MOVES)
        return;
    if (t && t != r->places[index].table && !make_room(r, t)) {
        drop_change(r, index, true);
        return;
    }
    if (r->apart && !room_joins(r, index))
        drop_change(r, index, true);
}

#endif
