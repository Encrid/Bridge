/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_STRIP_H
    #define BRIDGE_SRC_STORAGE_STRIP_H

    #include <string.h>

    #include "bits.h"
    #include "storage_bytes.h"
    #include "storage_layout.h"
    #include "storage_named.h"
    #include "storage_reach.h"
    #include "storage_table.h"
    #include "storage_walk.h"

static inline void close_bits(storage_t *r, uint32_t t, uint32_t id)
{
    const table_t *tab = &r->tables[t];
    uint8_t *shared = r->w->shared;
    uint32_t to = tab->block + (uint32_t)r->at[id];
    uint32_t bits = get_bits(to, tab->room);
    uint32_t from = get_bits(get_columns(r, t)[id], tab->room);

    memmove(shared + bits, shared + from, 8u * bits_words(tab->rows));
    memcpy(shared + to - COLUMN, &bits, 4);
}

static inline void close_column(storage_t *r, uint32_t t, uint32_t id)
{
    const table_t *tab = &r->tables[t];
    const storage_type_t *type = r->types[id];
    uint8_t *shared = r->w->shared;
    uint32_t *columns = get_columns(r, t);
    uint32_t to = tab->block + (uint32_t)r->at[id];

    if (type->kept == BRIDGE_TOGGLE)
        close_bits(r, t, id);
    memmove(shared + to, shared + columns[id], (size_t)type->size * tab->rows);
    columns[id] = to;
}

static inline void shorten_block(storage_t *r, uint32_t t, uint32_t gone)
{
    table_t *tab = &r->tables[t];
    const uint64_t *set = get_set(r, t);
    uint8_t *block = r->w->shared + tab->block;
    uint64_t len = lay_out(r, set, tab->room);
    walk_t walk = walk_set(set);
    uint32_t id = walk_next(r, &walk);

    while (id < r->type_room) {
        if (id > gone)
            close_column(r, t, id);
        id = walk_next(r, &walk);
    }
    (void)block_column_regrow(r->w, block, tab->block_len, (uint32_t)len);
    tab->block_len = (uint32_t)len;
}

static inline void strip_table(storage_t *r, uint32_t t,
    const storage_type_t *type, bool name)
{
    if (r->tables[t].named)
        unname_table(r, t);
    unlist_table(r, t);
    get_columns(r, t)[type->id] = 0;
    set_take(r->sets + (size_t)t * r->words, type->id);
    shorten_block(r, t, type->id);
    if (name)
        name_table(r, t);
    list_table(r, t);
    touch(r, t, true);
}

#endif
