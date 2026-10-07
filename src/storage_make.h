/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** src/storage_make.h -- a table made for a set that has none, and given
** back when its last row has left.
*/

#ifndef BRIDGE_SRC_STORAGE_MAKE_H
    #define BRIDGE_SRC_STORAGE_MAKE_H

    #include <string.h>

    #include "storage_named.h"
    #include "storage_reach.h"
    #include "storage_table.h"

/*
** close_table
**
** Makes a table no table: not live, with no row, no room, no block and no
** sparse set. What a barrier keeps of it for its own length stays.
*/
static inline void close_table(table_t *tab)
{
    tab->live = false;
    tab->rows = 0;
    tab->room = 0;
    tab->block = 0;
    tab->block_len = 0;
    tab->set = 0;
}

/*
** take_table_id
**
** Answers a table id nobody has: one given back, or the next never used.
** 0 if there is none.
*/
static inline uint32_t take_table_id(storage_t *r)
{
    uint32_t t = r->table_free;

    if (t) {
        r->table_free = r->table_next[t];
        return (t);
    }
    if (r->table_high > r->table_room)
        return (0);
    r->table_high++;
    return (r->table_high - 1);
}

/*
** open_table
**
** Makes table `t` the live table of a set, with no row and no column yet.
** Touched or not is kept: an id given back in this barrier is already on the
** list of what was touched.
*/
static inline void open_table(storage_t *r, uint32_t t, uint32_t slot,
    const uint64_t *set)
{
    r->tables[t] = (table_t){ .live = true, .touched = r->tables[t].touched };
    if (slot < r->set_room) {
        r->tables[t].set = slot + 1;
        r->set_table[slot] = t;
    } else {
        r->table_count++;
    }
    memcpy(r->sets + (size_t)t * r->words, set, 8u * r->words);
    memset(get_columns(r, t), 0, 4u * r->type_room);
    name_table(r, t);
    list_table(r, t);
    touch(r, t);
}

/*
** make_table
**
** Answers the table of a set, made if there is none. There is room for a
** table for each sparse set beside [budget] tables, so one of those is
** never refused.
*/
static inline uint32_t make_table(storage_t *r, const uint64_t *set)
{
    uint32_t t = find_table(r, set);
    uint32_t slot;

    if (t)
        return (t);
    slot = find_set_slot(r, set);
    if (slot == r->set_room && r->table_count >= r->b->tables)
        return (0);
    t = take_table_id(r);
    if (t)
        open_table(r, t, slot, set);
    return (t);
}

/*
** free_table
**
** Gives back a table whose last row has left.
*/
static inline void free_table(storage_t *r, uint32_t t)
{
    table_t *tab = &r->tables[t];

    if (tab->block)
        block_column_free(r->b, r->b->shared + tab->block, tab->block_len);
    if (tab->named)
        unname_table(r, t);
    unlist_table(r, t);
    memset(get_columns(r, t), 0, 4u * r->type_room);
    touch(r, t);
    if (tab->set)
        r->set_table[tab->set - 1] = 0;
    else
        r->table_count--;
    close_table(tab);
    r->table_next[t] = r->table_free;
    r->table_free = t;
}

#endif
