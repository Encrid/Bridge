/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_SETS_H
    #define BRIDGE_SRC_STORAGE_SETS_H

    #include "storage_inside.h"

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

static inline const uint64_t *get_set(const storage_t *r, uint32_t t)
{
    return (r->sets + (size_t)t * r->words);
}

#endif
