/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/storage_grow.h -- a table given room for more rows: a longer block,
** and each column moved out to where the new room puts it.
*/

#ifndef BRIDGE_SRC_STORAGE_GROW_H
    #define BRIDGE_SRC_STORAGE_GROW_H

    #include <string.h>

    #include "storage_bytes.h"
    #include "storage_layout.h"
    #include "storage_reach.h"
    #include "storage_table.h"
    #include "storage_view.h"
    #include "storage_walk.h"

/*
** take_longer
**
** Asks for a block of half as much room again as the table had, so that one
** that grows a row at a time is not moved each time, and for what was asked
** and no more where that is not to be had. Writes the room it got.
*/
static inline uint8_t *take_longer(storage_t *r, uint32_t t, uint32_t *room,
    uint64_t *len)
{
    const table_t *tab = &r->tables[t];
    uint32_t asked = *room;
    uint8_t *block;

    if (asked < tab->room + tab->room / 2)
        *room = tab->room + tab->room / 2;
    block = take_block(r, t, *room, len);
    if (block || *room == asked)
        return (block);
    *room = asked;
    return (take_block(r, t, asked, len));
}

/*
** move_column
**
** Moves one column out to where the new room puts it, which lay_out
** has just said. `was` and `had` are the block and the room the table had.
*/
static inline void move_column(storage_t *r, uint32_t t, uint32_t id,
    const table_t *old)
{
    const table_t *tab = &r->tables[t];
    const storage_type_t *type = r->types[id];
    uint8_t *block = r->b->shared + tab->block;
    uint32_t *columns = get_columns(r, t);
    uint32_t from = columns[id] - old->block;
    uint32_t bits = get_bits(columns[id], old->room) - old->block;
    size_t size = (size_t)type->size * tab->rows;

    memmove(block + r->at[id], block + from, size);
    if (type->kept == BRIDGE_TOGGLE)
        move_bits(r, t, id, bits);
    columns[id] = tab->block + (uint32_t)r->at[id];
}

/*
** move_columns
**
** Moves who is in which row to the new block, and every column out to where
** the new room puts it: the last first, so that none is written over before
** it is read. The columns are where the old room put them, in the new block.
*/
static inline void move_columns(storage_t *r, uint32_t t, const table_t *old)
{
    const table_t *tab = &r->tables[t];
    walk_t walk = walk_back(r, get_set(r, t));
    uint32_t id = walk_prev(r, &walk);

    if (tab->block != old->block)
        memcpy(get_who(r, tab), get_who(r, old), 8ull * tab->rows);
    while (id < r->type_room) {
        move_column(r, t, id, old);
        id = walk_prev(r, &walk);
    }
    if (tab->set)
        reseat_set(r, t);
}

/*
** place_columns
**
** Writes where each column of a table's first block is, which storage_lay_out
** has just said: nothing was anywhere before, so nothing moves.
*/
static inline void place_columns(storage_t *r, uint32_t t)
{
    const table_t *tab = &r->tables[t];
    uint32_t *columns = get_columns(r, t);
    walk_t walk = walk_set(get_set(r, t));
    uint32_t id = walk_next(r, &walk);

    while (id < r->type_room) {
        if (r->types[id]->kept == BRIDGE_TOGGLE)
            move_bits(r, t, id, 0);
        columns[id] = tab->block + (uint32_t)r->at[id];
        id = walk_next(r, &walk);
    }
}

/*
** grow_table
**
** Makes room for `room` rows in table `t`, or for half as much again as it
** had. Measured against twice: as fast, with half the idle room.
**
** storage.md, *A table*
*/
static inline bool grow_table(storage_t *r, uint32_t t, uint32_t room)
{
    table_t *tab = &r->tables[t];
    table_t old = *tab;
    uint64_t len;
    uint8_t *block = room > old.room ? take_longer(r, t, &room, &len) : nullptr;

    if (room <= old.room || !block)
        return (room <= old.room);
    tab->block = (uint32_t)(block - r->b->shared);
    tab->block_len = (uint32_t)len;
    tab->room = room;
    if (old.block)
        move_columns(r, t, &old);
    else
        place_columns(r, t);
    touch(r, t);
    return (true);
}

#endif
