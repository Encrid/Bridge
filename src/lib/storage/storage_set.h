/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_SET_H
    #define BRIDGE_SRC_STORAGE_SET_H

    #include "bits.h"
    #include "storage_inside.h"

static inline bool set_has(const uint64_t *set, uint32_t type)
{
    return (bits_has(set, type));
}

static inline void set_put(uint64_t *set, uint32_t type)
{
    bits_set(set, type);
}

static inline void set_take(uint64_t *set, uint32_t type)
{
    bits_clear(set, type);
}

static inline void set_copy(const storage_t *r, uint64_t *to,
    const uint64_t *from)
{
    uint32_t w = 0;

    while (w < r->words) {
        to[w] = from[w];
        w++;
    }
}

static inline void set_clear(const storage_t *r, uint64_t *set)
{
    uint32_t w = 0;

    while (w < r->words) {
        set[w] = 0;
        w++;
    }
}

#endif
