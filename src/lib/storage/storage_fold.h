/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_FOLD_H
    #define BRIDGE_SRC_STORAGE_FOLD_H

    #include "storage_carried.h"

static inline uint64_t *get_work(const storage_t *r, uint32_t index)
{
    return (r->work + (size_t)index * r->words);
}

static inline uint64_t *get_attached(const storage_t *r, uint32_t index)
{
    return (r->attached + (size_t)index * r->words);
}

static inline void start_fold(storage_t *r, uint32_t index)
{
    get_carried(r, index, get_work(r, index));
}

static inline void drop_change(storage_t *r, uint32_t index, bool for_columns)
{
    r->folded[index] |= for_columns ? DROPPED | SHORT : DROPPED;
}

static inline bool is_joining(const storage_t *r, uint32_t slot,
    uint32_t index)
{
    if (!r->set_type[slot] || get_where(r, slot)[index])
        return (false);
    return (set_has(get_work(r, index), r->set_type[slot]->id));
}

#endif
