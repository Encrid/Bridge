/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_WALK_H
    #define BRIDGE_SRC_STORAGE_WALK_H

    #include "storage_inside.h"

typedef struct {
    const uint64_t *set;
    uint64_t bits;
    uint32_t w;
} walk_t;

static inline walk_t walk_set(const uint64_t *set)
{
    return ((walk_t){ set, set[0], 0 });
}

[[gnu::always_inline]] static inline uint32_t walk_next(const storage_t *r,
    walk_t *walk)
{
    uint32_t id;

    while (!walk->bits && walk->w + 1 < r->words) {
        walk->w++;
        walk->bits = walk->set[walk->w];
    }
    if (!walk->bits)
        return (r->type_room);
    id = walk->w * 64 + (uint32_t)__builtin_ctzll(walk->bits);
    walk->bits &= walk->bits - 1;
    return (id);
}

static inline walk_t walk_back(const storage_t *r, const uint64_t *set)
{
    return ((walk_t){ set, set[r->words - 1], r->words - 1 });
}

[[gnu::always_inline]] static inline uint32_t walk_prev(const storage_t *r,
    walk_t *walk)
{
    uint32_t bit;

    while (!walk->bits && walk->w) {
        walk->w--;
        walk->bits = walk->set[walk->w];
    }
    if (!walk->bits)
        return (r->type_room);
    bit = 63 - (uint32_t)__builtin_clzll(walk->bits);
    walk->bits &= ~((uint64_t)1 << bit);
    return (walk->w * 64 + bit);
}

#endif
