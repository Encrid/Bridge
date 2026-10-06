/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/storage_reach.h -- where storage keeps what it knows of a table, a
** sparse set and a direct-indexed component, and what a module reads of
** each.
*/

#ifndef BRIDGE_SRC_STORAGE_REACH_H
    #define BRIDGE_SRC_STORAGE_REACH_H

    #include "storage_inside.h"

/*
** get_columns
**
** Answers where each type's column is in table `t`, by type: 0 if absent.
*/
static inline uint32_t *get_columns(const storage_t *r, uint32_t t)
{
    return (r->columns + (size_t)t * r->type_room);
}

/*
** get_where
**
** Answers, for each entity, its row in the sparse set of `slot` plus one.
*/
static inline uint32_t *get_where(const storage_t *r, uint32_t slot)
{
    return (r->where + (size_t)slot * r->b->entities);
}

/*
** get_cells
**
** Answers what a module reads of a sparse set: for each entity where its
** value is, from the shared base, and 0 where it has none.
**
** storage.md, *A sparse set*
*/
static inline uint32_t *get_cells(const storage_t *r, uint32_t slot)
{
    uint32_t *cells = (uint32_t *)(r->b->shared + r->b->sparse);

    return (cells + (size_t)slot * r->b->entities);
}

/*
** get_given
**
** Answers which entities have the direct-indexed component of `slot`: a bit
** an entity.
*/
static inline uint64_t *get_given(const storage_t *r, uint32_t slot)
{
    return (r->given + (size_t)slot * r->bit_words);
}

/*
** get_shown
**
** Answers what a module reads of the same: a bit an entity, where the
** sixteen bytes ahead of the cells say.
*/
static inline uint64_t *get_shown(const storage_t *r, uint32_t slot)
{
    uint64_t *shown = (uint64_t *)(r->b->shared + r->b->direct);

    return (shown + (size_t)slot * r->bit_words);
}

#endif
