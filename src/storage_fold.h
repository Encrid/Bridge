/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/storage_fold.h -- the fold: what each entity a barrier changes is
** becoming, kept beside where it still is.
*/

#ifndef BRIDGE_SRC_STORAGE_FOLD_H
    #define BRIDGE_SRC_STORAGE_FOLD_H

    #include "storage_carried.h"

/*
** get_work
**
** Answers the set of what the entity at `index` is becoming.
*/
static inline uint64_t *get_work(const storage_t *r, uint32_t index)
{
    return (r->work + (size_t)index * r->words);
}

/*
** get_attached
**
** Answers the set of what an attach wrote a value of, for the entity at
** `index`.
*/
static inline uint64_t *get_attached(const storage_t *r, uint32_t index)
{
    return (r->attached + (size_t)index * r->words);
}

/*
** start_fold
**
** Begins the fold of an entity from what it carries now.
*/
static inline void start_fold(storage_t *r, uint32_t index)
{
    get_carried(r, index, get_work(r, index));
}

/*
** drop_change
**
** Leaves an entity as it was, for want of a table or, `for_columns`, of
** columns: it is a failure of every module that asked a change of it, each
** toward its own budget, counted where the change's value is written.
**
** budgets.md, *What running out does*
*/
static inline void drop_change(storage_t *r, uint32_t index, bool for_columns)
{
    r->folded[index] |= for_columns ? DROPPED | SHORT : DROPPED;
}

/*
** is_joining
**
** Answers whether the entity at `index`, as the fold has it, is to join the
** sparse set of `slot`.
*/
static inline bool is_joining(const storage_t *r, uint32_t slot,
    uint32_t index)
{
    if (!r->set_type[slot] || get_where(r, slot)[index])
        return (false);
    return (set_has(get_work(r, index), r->set_type[slot]->id));
}

#endif
