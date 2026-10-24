/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_VIEW_H
    #define BRIDGE_SRC_STORAGE_VIEW_H

    #include "storage_bytes.h"

static inline uint64_t *get_who(const storage_t *r, const table_t *tab)
{
    return (r->who + (tab->block - r->w->columns) / 8);
}

static inline uint32_t get_handed(const storage_t *r, uint32_t type,
    uint32_t column)
{
    if (!r->tags || !column)
        return (column);
    if (r->types[type]->size || r->types[type]->kept == BRIDGE_TOGGLE)
        return (column);
    return (BRIDGE_CARRIED);
}

static inline uint32_t get_cell(const storage_type_t *type, uint32_t column,
    uint32_t row)
{
    if (!type->size)
        return (BRIDGE_CARRIED);
    return (column + row * type->size);
}

static inline bool is_on(const storage_t *r, uint32_t column, uint32_t room,
    uint32_t row)
{
    uint64_t word;

    memcpy(&word, r->w->shared + get_bits(column, room) + 8u * (row >> 6), 8);
    return (word >> (row & 63) & 1);
}

static inline void turn_row(storage_t *r, uint32_t bits, uint32_t row, bool on)
{
    uint8_t *at = r->w->shared + bits + 8u * (row >> 6);
    uint64_t bit = (uint64_t)1 << (row & 63);
    uint64_t word;

    memcpy(&word, at, 8);
    word = on ? word | bit : word & ~bit;
    memcpy(at, &word, 8);
}

#endif
