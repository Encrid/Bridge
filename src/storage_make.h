/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_MAKE_H
    #define BRIDGE_SRC_STORAGE_MAKE_H

    #include <string.h>

    #include "storage_named.h"
    #include "storage_reach.h"
    #include "storage_table.h"

static inline void close_table(table_t *tab)
{
    tab->live = false;
    tab->rows = 0;
    tab->room = 0;
    tab->block = 0;
    tab->block_len = 0;
    tab->set = 0;
}

static inline uint32_t take_table_id(storage_t *r)
{
    uint32_t t = r->table_free;

    if (t) {
        r->table_free = r->table_next[t];
        return (t);
    }
    r->table_high++;
    return (r->table_high - 1);
}

static inline void open_table(storage_t *r, uint32_t t, uint32_t slot,
    const uint64_t *set)
{
    r->tables[t] = (table_t){ .live = true, .touched = r->tables[t].touched };
    if (slot < r->set_room) {
        r->tables[t].set = slot + 1;
        r->set_table[slot] = t;
    } else {
        r->table_count++;
    }
    memcpy(r->sets + (size_t)t * r->words, set, 8u * r->words);
    memset(get_columns(r, t), 0, 4u * r->type_room);
    name_table(r, t);
    list_table(r, t);
    touch(r, t);
}

static inline uint32_t make_table(storage_t *r, const uint64_t *set)
{
    uint32_t t = find_table(r, set);
    uint32_t slot;

    if (t)
        return (t);
    slot = find_set_slot(r, set);
    if (slot == r->set_room && r->table_count >= r->b->tables)
        return (0);
    t = take_table_id(r);
    open_table(r, t, slot, set);
    return (t);
}

static inline void free_table(storage_t *r, uint32_t t)
{
    table_t *tab = &r->tables[t];

    if (tab->block)
        block_column_free(r->b, r->b->shared + tab->block, tab->block_len);
    if (tab->named)
        unname_table(r, t);
    unlist_table(r, t);
    memset(get_columns(r, t), 0, 4u * r->type_room);
    touch(r, t);
    if (tab->set)
        r->set_table[tab->set - 1] = 0;
    else
        r->table_count--;
    close_table(tab);
    r->table_next[t] = r->table_free;
    r->table_free = t;
}

#endif
