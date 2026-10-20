/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
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

static inline void move_column(storage_t *r, uint32_t t, uint32_t id,
    const table_t *old)
{
    const table_t *tab = &r->tables[t];
    const storage_type_t *type = r->types[id];
    uint8_t *block = r->w->shared + tab->block;
    uint32_t *columns = get_columns(r, t);
    uint32_t from = columns[id] - old->block;
    uint32_t bits = get_bits(columns[id], old->room) - old->block;
    size_t size = (size_t)type->size * tab->rows;

    memmove(block + r->at[id], block + from, size);
    if (type->kept == BRIDGE_TOGGLE)
        move_bits(r, t, id, bits);
    columns[id] = tab->block + (uint32_t)r->at[id];
}

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

static inline bool grow_table(storage_t *r, uint32_t t, uint32_t room)
{
    table_t *tab = &r->tables[t];
    table_t old = *tab;
    uint64_t len;
    uint8_t *block = room > old.room ? take_longer(r, t, &room, &len) : nullptr;

    if (room <= old.room || !block)
        return (room <= old.room);
    tab->block = (uint32_t)(block - r->w->shared);
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
