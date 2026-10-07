/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** src/storage_walk.h -- the types of a set walked, in the order of their
** numbers or against it.
**
** A walk keeps the word it is in and what is left of it, so that the next
** type is one count of zeros and one bit cleared. Asking a set again for
** "the next type after this one" was measured at 20 ns more for each table
** made, with nine types to a table.
*/

#ifndef BRIDGE_SRC_STORAGE_WALK_H
    #define BRIDGE_SRC_STORAGE_WALK_H

    #include "storage_inside.h"

typedef struct {
    const uint64_t *set;
    uint64_t bits;
    uint32_t w;
} walk_t;

/*
** walk_set
**
** Begins a walk of a set from its lowest type.
*/
static inline walk_t walk_set(const uint64_t *set)
{
    return ((walk_t){ set, set[0], 0 });
}

/*
** walk_next
**
** Answers the next type of the walk, or how many types there may be when
** there is none left.
*/
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

/*
** walk_back
**
** Begins a walk of a set from its highest type.
*/
static inline walk_t walk_back(const storage_t *r, const uint64_t *set)
{
    return ((walk_t){ set, set[r->words - 1], r->words - 1 });
}

/*
** walk_prev
**
** Answers the next type of a walk that goes from the highest down, or how
** many types there may be when there is none left.
*/
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
