/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/storage_view.h -- who is in a row, what a module is handed for a
** column, and a toggled row read and switched.
*/

#ifndef BRIDGE_SRC_STORAGE_VIEW_H
    #define BRIDGE_SRC_STORAGE_VIEW_H

    #include "storage_bytes.h"

/*
** get_who
**
** Answers which entity is in each row of a table: the column region's twin.
*/
static inline uint64_t *get_who(const storage_t *r, const table_t *tab)
{
    return (r->who + (tab->block - r->b->columns) / 8);
}

/*
** get_handed
**
** Answers what a module is handed for where a column is. A tag has none,
** and is handed that it is carried; but a toggled tag has its bits ahead of
** where its column would be, and is handed that. While no tag is declared,
** a column is where it is, and no type is looked at.
**
** storage.md, *A tag*
*/
static inline uint32_t get_handed(const storage_t *r, uint32_t type,
    uint32_t column)
{
    if (!r->tags || !column)
        return (column);
    if (r->types[type]->size || r->types[type]->kept == BRIDGE_TOGGLE)
        return (column);
    return (BRIDGE_CARRIED);
}

/*
** get_cell
**
** Answers what a module is handed for where the value of row `row` of a
** sparse set is.
*/
static inline uint32_t get_cell(const storage_type_t *type, uint32_t column,
    uint32_t row)
{
    if (!type->size)
        return (BRIDGE_CARRIED);
    return (column + row * type->size);
}

/*
** is_on
**
** Answers whether row `row` of the toggled column at `column` is on, in a
** block of `room` rows.
*/
static inline bool is_on(const storage_t *r, uint32_t column, uint32_t room,
    uint32_t row)
{
    uint64_t word;

    memcpy(&word, r->b->shared + get_bits(column, room) + 8u * (row >> 6), 8);
    return (word >> (row & 63) & 1);
}

/*
** turn_row
**
** Switches a row of a toggled column on or off. `bits` is where the
** column's bits are: get_bits says.
*/
static inline void turn_row(storage_t *r, uint32_t bits, uint32_t row, bool on)
{
    uint8_t *at = r->b->shared + bits + 8u * (row >> 6);
    uint64_t bit = (uint64_t)1 << (row & 63);
    uint64_t word;

    memcpy(&word, at, 8);
    word = on ? word | bit : word & ~bit;
    memcpy(at, &word, 8);
}

#endif
