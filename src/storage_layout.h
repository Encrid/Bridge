/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/storage_layout.h -- a table's block: its entity ids, then a column a
** type, each at a multiple of sixteen.
*/

#ifndef BRIDGE_SRC_STORAGE_LAYOUT_H
    #define BRIDGE_SRC_STORAGE_LAYOUT_H

    #include <string.h>

    #include "storage_bytes.h"
    #include "storage_reach.h"
    #include "storage_table.h"
    #include "storage_view.h"
    #include "storage_walk.h"

/*
** lay_out
**
** Answers how long a block of `room` rows of a set is, and writes into
** r->at where each of its columns begins in it. A tag has no column to begin
** anywhere, and a toggled column has its bits and sixteen bytes ahead of it.
*/
static inline uint64_t lay_out(const storage_t *r, const uint64_t *set,
    uint32_t room)
{
    const storage_type_t *type;
    uint64_t at = 8ull * room;
    walk_t walk = walk_set(set);
    uint32_t id = walk_next(r, &walk);

    while (id < r->type_room) {
        type = r->types[id];
        if (type->size || type->kept == BRIDGE_TOGGLE)
            at = pad_column(at);
        if (type->kept == BRIDGE_TOGGLE)
            at += get_bits_len(room) + COLUMN;
        r->at[id] = at;
        at += (uint64_t)type->size * room;
        id = walk_next(r, &walk);
    }
    return (pad_column(at));
}

/*
** reseat_set
**
** Writes again where each entity's value of a sparse set is, after the
** set's table moved.
*/
static inline void reseat_set(storage_t *r, uint32_t t)
{
    const table_t *tab = &r->tables[t];
    const storage_type_t *type = r->set_type[tab->set - 1];
    uint32_t column = get_columns(r, t)[type->id];
    uint32_t *cells = get_cells(r, tab->set - 1);
    const uint64_t *who = get_who(r, tab);
    uint32_t row = 0;

    while (row < tab->rows) {
        cells[(uint32_t)who[row]] = get_cell(type, column, row);
        row++;
    }
}

/*
** move_bits
**
** Moves the bits of a toggled column to where the new room puts them, after
** its values, since both move out; and writes ahead of the column where they
** are now. `from` is where the bits were in the block, or 0 for a new block.
*/
static inline void move_bits(storage_t *r, uint32_t t, uint32_t id,
    uint32_t from)
{
    const table_t *tab = &r->tables[t];
    uint8_t *block = r->b->shared + tab->block;
    uint32_t bits = tab->block + get_bits((uint32_t)r->at[id], tab->room);
    size_t len = 8u * ((tab->rows + 63) / 64);

    if (from)
        memmove(r->b->shared + bits, block + from, len);
    memcpy(block + r->at[id] - COLUMN, &bits, 4);
}

/*
** take_block
**
** Asks the column region for the block table `t` needs for `room` rows,
** grown from the one it has. Answers the block and writes how long it is, or
** nullptr with nothing changed.
*/
static inline uint8_t *take_block(storage_t *r, uint32_t t, uint32_t room,
    uint64_t *len)
{
    const table_t *tab = &r->tables[t];
    bridge_t *b = r->b;
    void *was = tab->block ? b->shared + tab->block : nullptr;

    *len = lay_out(r, get_set(r, t), room);
    if (*len > INT32_MAX)
        return (nullptr);
    return (block_column_regrow(b, was, tab->block_len, (uint32_t)*len));
}

#endif
