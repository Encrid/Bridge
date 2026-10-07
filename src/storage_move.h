/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/storage_move.h -- an entity moved from one table to another, a
** direct-indexed component given and taken, and a singleton set.
*/

#ifndef BRIDGE_SRC_STORAGE_MOVE_H
    #define BRIDGE_SRC_STORAGE_MOVE_H

    #include "storage_row.h"
    #include "storage_set.h"

/*
** set_single
**
** Makes a singleton have a value, of zero until one is written, or have
** none: and every module is written for again, since where it is reached
** says which.
*/
static inline void set_single(storage_t *r, storage_type_t *type, bool set)
{
    type->set = set;
    memset(r->b->shared + type->cells, 0, type->size);
    r->b->layout_moved = 1;
}

/*
** give_cell
**
** Gives the entity at `index` the direct-indexed component of `slot`, or
** takes it: its bit, here and where modules read it, and its cell made zero
** either way.
*/
static inline void give_cell(storage_t *r, uint32_t slot, uint32_t index,
    bool on)
{
    const storage_type_t *type = r->direct_type[slot];
    uint8_t *cells = r->b->shared + type->cells + COLUMN;

    if (on) {
        set_put(get_given(r, slot), index);
        set_put(get_shown(r, slot), index);
    } else {
        set_take(get_given(r, slot), index);
        set_take(get_shown(r, slot), index);
    }
    clear_bytes(cells + (size_t)index * type->size, type->size);
}

/*
** carry_value
**
** Writes one column of a row an entity moves into: what it carried of that
** type goes with it, and what it did not is zero. A toggled component is on
** when it is given, and as it was when it moves.
*/
[[gnu::always_inline]] static inline void carry_value(storage_t *r,
    const row_move_t *move, uint32_t id)
{
    uint32_t size = r->types[id]->size;
    uint32_t into = get_columns(r, move->to)[id];
    uint32_t out = move->from ? get_columns(r, move->from)[id] : 0;
    uint8_t *shared = r->b->shared;
    uint8_t *cell = shared + into + (size_t)move->at * size;
    bool on = !out;

    if (out)
        copy_bytes(cell, shared + out + (size_t)move->row * size, size);
    else
        clear_bytes(cell, size);
    if (r->types[id]->kept != BRIDGE_TOGGLE)
        return;
    if (out)
        on = is_on(r, out, r->tables[move->from].room, move->row);
    turn_row(r, get_bits(into, r->tables[move->to].room), move->at, on);
}

/*
** carry_row
**
** Writes every column of the row an entity moves into.
*/
[[gnu::always_inline]] static inline void carry_row(storage_t *r,
    const row_move_t *move)
{
    walk_t walk = walk_set(get_set(r, move->to));
    uint32_t id = walk_next(r, &walk);

    while (id < r->type_room) {
        carry_value(r, move, id);
        id = walk_next(r, &walk);
    }
}

/*
** move_row
**
** Moves the entity at `index` to table `to`, which has room: what it carried
** that `to` also holds goes with it, and what `to` holds that it did not is
** zero.
*/
[[gnu::always_inline]] static inline void move_row(storage_t *r,
    uint32_t index, uint32_t to)
{
    const place_t *p = &r->places[index];
    row_move_t move = { index, p->table, p->row, to, 0 };

    if (move.from == to)
        return;
    if (to) {
        move.at = append_row(r, to, index);
        carry_row(r, &move);
    }
    if (move.from)
        remove_row(r, move.from, move.row);
    place_row(r, index, to, move.at);
}

#endif
