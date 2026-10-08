/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/storage_reserve.h -- a barrier's second step, and a move's: a table
** for each changed entity, before anything moves.
*/

#ifndef BRIDGE_SRC_STORAGE_RESERVE_H
    #define BRIDGE_SRC_STORAGE_RESERVE_H

    #include "storage_fold.h"
    #include "storage_make.h"

/*
** get_wanted
**
** Answers the set of the table the entity at `index` is to be in: what it
** will carry that is kept in tables, and no more.
*/
[[gnu::always_inline]] static inline const uint64_t *get_wanted(storage_t *r,
    uint32_t index)
{
    const uint64_t *want = get_work(r, index);
    uint32_t w = 0;

    if (!r->apart)
        return (want);
    while (w < r->words) {
        r->masked[w] = want[w] & r->in_table[w];
        w++;
    }
    return (r->masked);
}

/*
** pick_table
**
** Finds the table the entity at `index` is to be in, made if there is none,
** and counts the entity among the rows that table gains. What it carries
** already, it carries where it is: an attach of what it has is a value and
** no move, in whichever table of that set it happens to be. false if there
** is no room for the table, and the entity is left as it was.
*/
[[gnu::always_inline]] static inline bool pick_table(storage_t *r,
    uint32_t index)
{
    const uint64_t *want = get_wanted(r, index);
    uint32_t from = r->places[index].table;
    uint32_t t = 0;

    if (from && set_is_same(r, want, get_set(r, from))) {
        t = from;
    } else if (!set_is_empty(r, want)) {
        t = make_table(r, want);
        if (!t) {
            drop_change(r, index, false);
            return (false);
        }
    }
    r->dest[index] = t;
    if (t && t != from) {
        r->tables[t].incoming++;
        r->folded[index] |= MOVES;
    }
    return (true);
}

/*
** reserve_join
**
** Counts the entity at `index` among the rows the sparse set of `slot`
** gains, with the set's table made if no entity was in it. There is room
** for a table for each sparse set, so that one is never refused.
*/
static inline void reserve_join(storage_t *r, uint32_t slot, uint32_t index)
{
    if (!r->set_table[slot]) {
        set_clear(r, r->scratch);
        set_put(r->scratch, r->set_type[slot]->id);
        if (!make_table(r, r->scratch))
            unreachable();
    }
    r->tables[r->set_table[slot]].incoming++;
    r->folded[index] |= MOVES;
}

/*
** count_joins
**
** Counts a row in each sparse set the entity at `index` is to join.
*/
static inline void count_joins(storage_t *r, uint32_t index)
{
    uint32_t slot = 0;

    while (slot < r->set_room) {
        if (is_joining(r, slot, index))
            reserve_join(r, slot, index);
        slot++;
    }
}

/*
** reserve_one
**
** Finds where the entity at `index` is to be, while it is still where it
** was: its table, and each sparse set it is to join.
*/
[[gnu::always_inline]] static inline void reserve_one(storage_t *r,
    uint32_t index)
{
    if (r->folded[index] & DESTROYED)
        return;
    if (pick_table(r, index) && r->apart)
        count_joins(r, index);
}

#endif
