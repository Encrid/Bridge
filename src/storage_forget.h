/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/storage_forget.h -- the end of a barrier's work on rows: the fold
** forgotten, and each table nothing is left in given back.
*/

#ifndef BRIDGE_SRC_STORAGE_FORGET_H
    #define BRIDGE_SRC_STORAGE_FORGET_H

    #include "storage_make.h"

static inline void unsize_table(storage_t *r, uint32_t t)
{
    r->tables[t].incoming = 0;
    r->tables[t].sized = false;
}

/*
** forget_changed
**
** Forgets what the fold decided of each changed entity, and the room that
** was found for it.
*/
static inline void forget_changed(storage_t *r)
{
    uint32_t index;
    uint32_t i = 0;

    while (i < r->changed_count) {
        index = r->changed[i];
        if (r->dest[index])
            unsize_table(r, r->dest[index]);
        r->folded[index] = 0;
        r->dest[index] = 0;
        i++;
    }
    r->changed_count = 0;
}

static inline void forget_sets(storage_t *r)
{
    uint32_t slot = 0;

    while (slot < r->set_room) {
        if (r->set_table[slot])
            unsize_table(r, r->set_table[slot]);
        slot++;
    }
}

/*
** forget_all
**
** Forgets the fold, and gives back each table that was touched and that
** nothing is left in.
*/
static inline void forget_all(storage_t *r)
{
    uint32_t t;
    uint32_t i = 0;

    forget_changed(r);
    forget_sets(r);
    while (i < r->dirty_count) {
        t = r->dirty[i];
        if (r->tables[t].live && !r->tables[t].rows)
            free_table(r, t);
        i++;
    }
}

#endif
