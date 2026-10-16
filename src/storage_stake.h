/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_STAKE_H
    #define BRIDGE_SRC_STORAGE_STAKE_H

    #include "storage_fold.h"

static inline void set_write(uint64_t *set, uint32_t type, bool put)
{
    if (put)
        set_put(set, type);
    else
        set_take(set, type);
}

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

[[gnu::always_inline]] static inline void fold_destroy(storage_t *r,
    module_t *m, uint32_t index)
{
    if (has_stake(r, m, index))
        r->folded[index] |= DESTROYED;
    else
        m->barred++;
}

#endif
