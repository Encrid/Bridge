/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/storage_sets.h -- a set of types compared, hashed and walked.
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
** next_type
**
** Answers the lowest type of a set that is `from` or above, or how many
** types there may be if there is none: how a set's types are walked, in the
** order of their numbers.
*/
[[gnu::always_inline]] static inline uint32_t next_type(const storage_t *r,
    const uint64_t *set, uint32_t from)
{
    uint32_t w = from >> 6;
    uint64_t bits = 0;

    if (w < r->words)
        bits = set[w] & (~0ull << (from & 63));
    while (!bits && w + 1 < r->words) {
        w++;
        bits = set[w];
    }
    if (!bits)
        return (r->type_room);
    return (w * 64 + (uint32_t)__builtin_ctzll(bits));
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
