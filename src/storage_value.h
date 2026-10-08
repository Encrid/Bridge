/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/storage_value.h -- where an entity has its value of a type, in
** whichever storage layout the type is kept; and a sparse set joined and
** left.
*/

#ifndef BRIDGE_SRC_STORAGE_VALUE_H
    #define BRIDGE_SRC_STORAGE_VALUE_H

    #include "storage_row.h"
    #include "storage_set.h"

/*
** get_sparse_value
**
** Answers where the entity at `index` has its value in a sparse set, from
** the shared base: 0 if it is not in it.
*/
static inline uint32_t get_sparse_value(const storage_t *r, uint32_t index,
    const storage_type_t *type)
{
    uint32_t row = get_where(r, type->slot)[index];
    uint32_t t = r->set_table[type->slot];

    if (!row)
        return (0);
    return (get_columns(r, t)[type->id] + (row - 1) * type->size);
}

/*
** get_table_value
**
** Answers where the entity at `index` has its value in its table: 0 if it
** is in none, or its table has no column of the type.
*/
static inline uint32_t get_table_value(const storage_t *r, uint32_t index,
    const storage_type_t *type)
{
    uint32_t t = r->places[index].table;
    uint32_t column = t ? get_columns(r, t)[type->id] : 0;

    if (!column)
        return (0);
    return (column + r->places[index].row * type->size);
}

/*
** get_value
**
** Answers where the entity at `index` has its value of a type, from the
** shared base: 0 if it has none.
*/
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

/*
** join_set
**
** Has the entity at `index` join the sparse set of `slot`, whose table has
** room for it: with a value of zero, until an attach's is written.
*/
static inline void join_set(storage_t *r, uint32_t slot, uint32_t index)
{
    const storage_type_t *type = r->set_type[slot];
    uint32_t t = r->set_table[slot];
    uint32_t row = append_row(r, t, index);
    uint8_t *column = r->b->shared + get_columns(r, t)[type->id];

    clear_bytes(column + (size_t)row * type->size, type->size);
    seat_row(r, &r->tables[t], index, row);
}

/*
** leave_set
**
** Has the entity at `index` leave the sparse set of `slot`: the set's last
** row takes its place.
*/
static inline void leave_set(storage_t *r, uint32_t slot, uint32_t index)
{
    uint32_t row = get_where(r, slot)[index] - 1;

    get_where(r, slot)[index] = 0;
    get_cells(r, slot)[index] = 0;
    remove_row(r, r->set_table[slot], row);
}

#endif
