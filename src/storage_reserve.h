/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_RESERVE_H
    #define BRIDGE_SRC_STORAGE_RESERVE_H

    #include "storage_fold.h"
    #include "storage_make.h"

[[gnu::always_inline]] static inline const uint64_t *get_wanted(storage_t *r,
    uint32_t index)
{
    const uint64_t *want = get_work(r, index);
    uint32_t w = 0;

    if (!r->apart)
        return (want);
    while (w < r->words) {
        r->masked[w] = want[w] & r->in_table[w];
        w++;
    }
    return (r->masked);
}

[[gnu::always_inline]] static inline bool pick_table(storage_t *r,
    uint32_t index)
{
    const uint64_t *want = get_wanted(r, index);
    uint32_t from = r->places[index].table;
    uint32_t t = 0;

    if (from && set_is_same(r, want, get_set(r, from))) {
        t = from;
    } else if (!set_is_empty(r, want)) {
        t = make_table(r, want);
        if (!t) {
            drop_change(r, index, false);
            return (false);
        }
    }
    r->dest[index] = t;
    if (t && t != from) {
        r->tables[t].incoming++;
        r->folded[index] |= MOVES;
    }
    return (true);
}

static inline void reserve_join(storage_t *r, uint32_t slot, uint32_t index)
{
    if (!r->set_table[slot]) {
        set_clear(r, r->scratch);
        set_put(r->scratch, r->set_type[slot]->id);
        if (!make_table(r, r->scratch))
            unreachable();
    }
    r->tables[r->set_table[slot]].incoming++;
    r->folded[index] |= MOVES;
}

static inline void count_joins(storage_t *r, uint32_t index)
{
    uint32_t slot = 0;

    while (slot < r->set_room) {
        if (is_joining(r, slot, index))
            reserve_join(r, slot, index);
        slot++;
    }
}

[[gnu::always_inline]] static inline void reserve_one(storage_t *r,
    uint32_t index)
{
    if (r->folded[index] & DESTROYED)
        return;
    if (pick_table(r, index) && r->apart)
        count_joins(r, index);
}

#endif
