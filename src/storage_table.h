/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/storage_table.h -- a table found by its set, matched against terms,
** and marked touched.
*/

#ifndef BRIDGE_SRC_STORAGE_TABLE_H
    #define BRIDGE_SRC_STORAGE_TABLE_H

    #include "storage_set.h"
    #include "storage_sets.h"

/*
** touch
**
** Marks a table touched since what modules read was last written.
*/
static inline void touch(storage_t *r, uint32_t t)
{
    if (r->tables[t].touched)
        return;
    r->tables[t].touched = true;
    r->dirty[r->dirty_count] = t;
    r->dirty_count++;
}

/*
** find_table
**
** Answers the table a set is found as, or 0.
*/
static inline uint32_t find_table(const storage_t *r, const uint64_t *set)
{
    uint32_t i = hash_set(r, set);

    while (r->hash[i] && !set_is_same(r, get_set(r, r->hash[i]), set))
        i = (i + 1) & (r->hash_room - 1);
    return (r->hash[i]);
}

/*
** terms_match
**
** Answers whether a set carries every type the terms must have and none
** they must not.
*/
static inline bool terms_match(const storage_terms_t *t, const uint64_t *set)
{
    uint32_t i = 0;
    bool carried;

    while (i < t->count) {
        carried = set_has(set, t->type[i]);
        if (t->oper[i] == BRIDGE_AND && !carried)
            return (false);
        if (t->oper[i] == BRIDGE_NOT && carried)
            return (false);
        i++;
    }
    return (true);
}

/*
** terms_read
**
** Answers whether a system's spans include table `t`: its set matches, and a
** sparse set's table is walked only by a system that must have its type.
**
** storage.md, *What a query walks*
*/
static inline bool terms_read(const storage_t *r, const storage_terms_t *s,
    uint32_t t)
{
    uint32_t set = r->tables[t].set;
    uint32_t type = set ? r->set_type[set - 1]->id : 0;
    uint32_t i = 0;

    if (!terms_match(s, get_set(r, t)))
        return (false);
    if (!set)
        return (true);
    while (i < s->count && (s->oper[i] != BRIDGE_AND || s->type[i] != type))
        i++;
    return (i < s->count);
}

/*
** find_set_slot
**
** Answers which sparse set has a type in a set, or how many there may be if
** none has: a set that holds a type kept in a sparse set is that sparse
** set's table.
*/
static inline uint32_t find_set_slot(const storage_t *r, const uint64_t *set)
{
    uint32_t slot = 0;

    while (slot < r->set_room) {
        if (r->set_type[slot] && set_has(set, r->set_type[slot]->id))
            return (slot);
        slot++;
    }
    return (slot);
}

#endif
