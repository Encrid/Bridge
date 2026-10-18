/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_REACH_H
    #define BRIDGE_SRC_STORAGE_REACH_H

    #include "storage_inside.h"

static inline uint32_t *get_columns(const storage_t *r, uint32_t t)
{
    return (r->columns + (size_t)t * r->type_room);
}

static inline uint32_t *get_where(const storage_t *r, uint32_t slot)
{
    return (r->where + (size_t)slot * r->b->entities);
}

static inline uint32_t *get_cells(const storage_t *r, uint32_t slot)
{
    uint32_t *cells = (uint32_t *)(r->b->shared + r->b->sparse);

    return (cells + (size_t)slot * r->b->entities);
}

static inline uint64_t *get_given(const storage_t *r, uint32_t slot)
{
    return (r->given + (size_t)slot * r->bit_words);
}

static inline uint64_t *get_shown(const storage_t *r, uint32_t slot)
{
    uint64_t *shown = (uint64_t *)(r->b->shared + r->b->direct);

    return (shown + (size_t)slot * r->bit_words);
}

#endif
