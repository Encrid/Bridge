/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** src/storage_sets.h -- a set of types compared and hashed, and a table's set
** found.
*/

#ifndef BRIDGE_SRC_STORAGE_SETS_H
    #define BRIDGE_SRC_STORAGE_SETS_H

    #include "storage_inside.h"

/*
** set_is_same
**
** Answers whether two sets hold the same types. Compared a word at a time,
** since memcmp of a length the compiler cannot see is a call.
*/
static inline bool set_is_same(const storage_t *r, const uint64_t *a,
    const uint64_t *b)
{
    uint32_t w = 0;

    while (w < r->words && a[w] == b[w])
        w++;
    return (w == r->words);
}

static inline bool set_is_empty(const storage_t *r, const uint64_t *set)
{
    uint32_t w = 0;

    while (w < r->words && !set[w])
        w++;
    return (w == r->words);
}

/*
** hash_set
**
** Answers where a set is looked for first in the table of tables.
*/
static inline uint32_t hash_set(const storage_t *r, const uint64_t *set)
{
    uint64_t h = 1'469'598'103'934'665'603ull;
    uint32_t w = 0;

    while (w < r->words) {
        h = (h ^ set[w]) * 1'099'511'628'211ull;
        w++;
    }
    return ((uint32_t)(h ^ h >> 32) & (r->hash_room - 1));
}

/*
** get_set
**
** Answers which types the entities of table `t` carry.
*/
static inline const uint64_t *get_set(const storage_t *r, uint32_t t)
{
    return (r->sets + (size_t)t * r->words);
}

#endif
