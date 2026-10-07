/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/storage_set.h -- a set of types: a bit a type, in 64-bit words.
**
** A set is a word for each 64 types there may be: one, in most deployments.
** It is copied a word at a time here, since memcpy of a length the compiler
** cannot see is a call, and this is made for every entity a barrier changes.
*/

#ifndef BRIDGE_SRC_STORAGE_SET_H
    #define BRIDGE_SRC_STORAGE_SET_H

    #include "storage_inside.h"

static inline bool set_has(const uint64_t *set, uint32_t type)
{
    return (set[type >> 6] >> (type & 63) & 1);
}

static inline void set_put(uint64_t *set, uint32_t type)
{
    set[type >> 6] |= (uint64_t)1 << (type & 63);
}

static inline void set_take(uint64_t *set, uint32_t type)
{
    set[type >> 6] &= ~((uint64_t)1 << (type & 63));
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
