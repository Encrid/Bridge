/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_VALUE_H
    #define BRIDGE_SRC_STORAGE_VALUE_H

    #include "storage_row.h"
    #include "storage_set.h"

static inline uint32_t get_sparse_value(const storage_t *r, uint32_t index,
    const storage_type_t *type)
{
    uint32_t row = get_where(r, type->slot)[index];
    uint32_t t = r->set_table[type->slot];

    if (!row)
        return (0);
    return (get_columns(r, t)[type->id] + (row - 1) * type->size);
}

static inline uint32_t get_table_value(const storage_t *r, uint32_t index,
    const storage_type_t *type)
{
    uint32_t t = r->places[index].table;
    uint32_t column = t ? get_columns(r, t)[type->id] : 0;

    if (!column)
        return (0);
    return (column + r->places[index].row * type->size);
}

[[gnu::always_inline]] static inline uint32_t get_value(const storage_t *r,
    uint32_t index, const storage_type_t *type)
{
    if (type->kept == BRIDGE_TABLE || type->kept == BRIDGE_TOGGLE)
        return (get_table_value(r, index, type));
    if (type->kept == BRIDGE_SPARSE_SET)
        return (get_sparse_value(r, index, type));
    if (type->kept == BRIDGE_SINGLETON)
        return (type->set ? type->cells : 0);
    if (!set_has(get_given(r, type->slot), index))
        return (0);
    return (type->cells + (uint32_t)COLUMN + index * type->size);
}

static inline void join_set(storage_t *r, uint32_t slot, uint32_t index)
{
    const storage_type_t *type = r->set_type[slot];
    uint32_t t = r->set_table[slot];
    uint32_t row = append_row(r, t, index);
    uint8_t *column = r->w->shared + get_columns(r, t)[type->id];

    clear_bytes(column + (size_t)row * type->size, type->size);
    seat_row(r, &r->tables[t], index, row);
}

static inline void leave_set(storage_t *r, uint32_t slot, uint32_t index)
{
    uint32_t row = get_where(r, slot)[index] - 1;

    get_where(r, slot)[index] = 0;
    get_cells(r, slot)[index] = 0;
    remove_row(r, r->set_table[slot], row);
}

#endif
