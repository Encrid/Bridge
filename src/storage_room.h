/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/storage_room.h -- room found, before anything moves, for every row a
** barrier adds to a table. What cannot be had is known here, while every
** entity is still where it was.
**
** budgets.md, *What running out does*
*/

#ifndef BRIDGE_SRC_STORAGE_ROOM_H
    #define BRIDGE_SRC_STORAGE_ROOM_H

    #include "storage_grow.h"
    #include "storage_reserve.h"

/*
** size_table
**
** Finds room in table `t` for all the rows it is to gain at this barrier, or
** for as many as the column region is found to hold: by halves down to a
** number that fits, and from there back up between it and the last that did
** not.
*/
static inline void size_table(storage_t *r, uint32_t t)
{
    table_t *tab = &r->tables[t];
    uint32_t most = tab->incoming;
    uint32_t between;

    while (tab->incoming && !grow_table(r, t, tab->rows + tab->incoming))
        tab->incoming /= 2;
    while (most - tab->incoming > 1) {
        between = tab->incoming + (most - tab->incoming) / 2;
        if (grow_table(r, t, tab->rows + between))
            tab->incoming = between;
        else
            most = between;
    }
    tab->sized = true;
}

/*
** make_room
**
** Takes room for one more of the rows table `t` is to gain at this barrier.
** The first time it is asked of a table, room is found for all of them.
** false if there is none left.
*/
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

/*
** room_joins
**
** Takes room in each sparse set the entity at `index` is to join. false at
** the first that has none.
*/
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

/*
** room_one
**
** Takes room for the entity at `index` where it is to be, or leaves it as it
** was for want of columns. One that is to have no row it has not is asked
** nothing: an attach of what an entity carries is most of what a barrier
** sees.
*/
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

/*
** reserve_all
**
** A barrier's second step, and a move's: a table for each changed entity,
** and room in it and in each sparse set it is to join, before anything
** moves.
*/
[[gnu::always_inline]] static inline void reserve_all(storage_t *r)
{
    uint32_t i = 0;

    while (i < r->changed_count) {
        reserve_one(r, r->changed[i]);
        i++;
    }
    i = 0;
    while (i < r->changed_count) {
        room_one(r, r->changed[i]);
        i++;
    }
}

#endif
