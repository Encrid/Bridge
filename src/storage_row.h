/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** src/storage_row.h -- a row of a table: added at its end, and taken out
** with the last one moved into its place.
*/

#ifndef BRIDGE_SRC_STORAGE_ROW_H
    #define BRIDGE_SRC_STORAGE_ROW_H

    #include "storage_bytes.h"
    #include "storage_reach.h"
    #include "storage_table.h"
    #include "storage_view.h"
    #include "storage_walk.h"

/*
** place_row
**
** Records that the entity at `index` is in row `row` of table `t`: here, and
** in the entity index modules read.
*/
static inline void place_row(storage_t *r, uint32_t index, uint32_t t,
    uint32_t row)
{
    place_t *p = &r->places[index];

    p->table = t;
    p->row = row;
    r->b->index[index] = (bridge_entity_t){ t, row, p->generation };
}

/*
** seat_row
**
** Records that the entity at `index` has row `row` of a sparse set's table:
** kept here, and written where modules read it.
*/
static inline void seat_row(storage_t *r, const table_t *tab, uint32_t index,
    uint32_t row)
{
    uint32_t slot = tab->set - 1;
    const storage_type_t *type = r->set_type[slot];
    uint32_t column = get_columns(r, (uint32_t)(tab - r->tables))[type->id];

    get_where(r, slot)[index] = row + 1;
    get_cells(r, slot)[index] = get_cell(type, column, row);
}

/*
** append_row
**
** Adds a row at the end of table `t`, which has room for it, for the entity
** at `index`. Answers the row.
*/
static inline uint32_t append_row(storage_t *r, uint32_t t, uint32_t index)
{
    table_t *tab = &r->tables[t];
    uint64_t id = (uint64_t)r->places[index].generation << 32 | index;
    uint32_t row = tab->rows;

    tab->rows++;
    get_who(r, tab)[row] = id;
    memcpy(r->b->shared + tab->block + 8ull * row, &id, 8);
    touch(r, t);
    return (row);
}

/*
** fill_row
**
** Copies the last row of a table into row `row`, a column at a time, with
** whether each toggled column is on.
*/
[[gnu::always_inline]] static inline void fill_row(storage_t *r, uint32_t t,
    uint32_t row, uint32_t last)
{
    uint32_t room = r->tables[t].room;
    const uint32_t *columns = get_columns(r, t);
    walk_t walk = walk_set(get_set(r, t));
    uint32_t id = walk_next(r, &walk);
    size_t size;
    uint8_t *column;

    while (id < r->type_room) {
        size = r->types[id]->size;
        column = r->b->shared + columns[id];
        copy_bytes(column + row * size, column + last * size, (uint32_t)size);
        if (r->types[id]->kept == BRIDGE_TOGGLE)
            turn_row(r, get_bits(columns[id], room), row,
                is_on(r, columns[id], room, last));
        id = walk_next(r, &walk);
    }
}

/*
** remove_row
**
** Takes a row out of table `t`: the last one is moved into its place, and
** the entity that was in it is told where it is now.
*/
[[gnu::always_inline]] static inline void remove_row(storage_t *r, uint32_t t,
    uint32_t row)
{
    table_t *tab = &r->tables[t];
    uint64_t *who = get_who(r, tab);
    uint32_t last = tab->rows - 1;

    tab->rows = last;
    touch(r, t);
    if (row == last)
        return;
    fill_row(r, t, row, last);
    who[row] = who[last];
    memcpy(r->b->shared + tab->block + 8ull * row, &who[row], 8);
    if (tab->set)
        seat_row(r, tab, (uint32_t)who[row], row);
    else
        place_row(r, (uint32_t)who[row], t, row);
}

#endif
