/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_LAYOUT_H
    #define BRIDGE_SRC_STORAGE_LAYOUT_H

    #include <string.h>

    #include "storage_bytes.h"
    #include "storage_reach.h"
    #include "storage_table.h"
    #include "storage_view.h"
    #include "storage_walk.h"

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

static inline void move_bits(storage_t *r, uint32_t t, uint32_t id,
    uint32_t from)
{
    const table_t *tab = &r->tables[t];
    uint8_t *block = r->w->shared + tab->block;
    uint32_t bits = tab->block + get_bits((uint32_t)r->at[id], tab->room);
    size_t len = 8u * ((tab->rows + 63) / 64);

    if (from)
        memmove(r->w->shared + bits, block + from, len);
    memcpy(block + r->at[id] - COLUMN, &bits, 4);
}

static inline uint8_t *take_block(storage_t *r, uint32_t t, uint32_t room,
    uint64_t *len)
{
    const table_t *tab = &r->tables[t];
    bridge_world_t *w = r->w;
    void *was = tab->block ? w->shared + tab->block : nullptr;

    *len = lay_out(r, get_set(r, t), room);
    if (*len > INT32_MAX)
        return (nullptr);
    return (block_column_regrow(w, was, tab->block_len, (uint32_t)*len));
}

#endif
