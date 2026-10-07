/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** src/storage_stake.h -- what a module may destroy, and a type put in a set
** or taken out by one flag.
*/

#ifndef BRIDGE_SRC_STORAGE_STAKE_H
    #define BRIDGE_SRC_STORAGE_STAKE_H

    #include "storage_fold.h"

/*
** set_write
**
** Puts a type in a set, or takes it out.
*/
static inline void set_write(uint64_t *set, uint32_t type, bool put)
{
    if (put)
        set_put(set, type);
    else
        set_take(set, type);
}

/*
** has_apart_stake
**
** Answers whether a module may destroy the entity at `index`, where some
** type is kept apart from tables: from everything the entity carries.
*/
static inline bool has_apart_stake(const storage_t *r, const module_t *m,
    uint32_t index)
{
    uint64_t *carried = r->scratch;
    uint64_t any = 0;
    uint64_t written = 0;
    uint32_t w = 0;

    get_carried(r, index, carried);
    while (w < r->words) {
        any |= carried[w];
        if (m->writable)
            written |= carried[w] & m->writable[w];
        w++;
    }
    return (written || !any);
}

/*
** has_stake
**
** Answers whether a module may destroy the entity at `index`: it carries
** something the module may write, or it carries nothing. Read from where the
** entity is, before this barrier moves it. While no type is kept apart from
** tables, what it carries is what its table holds.
**
** grants.md, *What a module may destroy*
*/
[[gnu::always_inline]] static inline bool has_stake(const storage_t *r,
    const module_t *m, uint32_t index)
{
    uint32_t t = r->places[index].table;
    const uint64_t *set = get_set(r, t);
    uint64_t written = 0;
    uint32_t w = 0;

    if (r->apart)
        return (has_apart_stake(r, m, index));
    while (t && m->writable && w < r->words) {
        written |= set[w] & m->writable[w];
        w++;
    }
    return (written || !t);
}

/*
** fold_destroy
**
** Folds a destroy: the entity is gone at this barrier if the module that
** asked has a stake in it, and otherwise the destroy is barred, and counted
** against the module.
*/
[[gnu::always_inline]] static inline void fold_destroy(storage_t *r,
    module_t *m, uint32_t index)
{
    if (has_stake(r, m, index))
        r->folded[index] |= DESTROYED;
    else
        m->barred++;
}

#endif
