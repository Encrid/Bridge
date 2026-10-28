/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_HUSK_H
    #define BRIDGE_SRC_STORAGE_HUSK_H

    #include "storage_fold.h"

static inline bool is_husk(const storage_t *r, uint32_t index)
{
    return (set_has(r->husks, index) || r->folded[index] & HUSKED);
}

static inline void fold_husk(storage_t *r, uint32_t index)
{
    uint64_t *work = get_work(r, index);

    set_mask(r, work, work, r->husk_types);
    r->folded[index] |= set_is_empty(r, work) ? DESTROYED : HUSKED;
}

static inline void end_husk(storage_t *r, uint32_t index)
{
    if (set_is_empty(r, get_work(r, index)) && is_husk(r, index))
        r->folded[index] |= DESTROYED;
}

#endif
