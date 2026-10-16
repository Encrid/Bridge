/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_STORAGE_BYTES_H
    #define BRIDGE_SRC_STORAGE_BYTES_H

    #include <string.h>

    #include "storage_inside.h"

static inline void copy_bytes(uint8_t *to, const uint8_t *from, uint32_t size)
{
    if (size == 4)
        memcpy(to, from, 4);
    else if (size == 8)
        memcpy(to, from, 8);
    else
        memcpy(to, from, size);
}

static inline void clear_bytes(uint8_t *at, uint32_t size)
{
    if (size == 4)
        memset(at, 0, 4);
    else if (size == 8)
        memset(at, 0, 8);
    else
        memset(at, 0, size);
}

static inline uint64_t pad_column(uint64_t bytes)
{
    return ((bytes + COLUMN - 1) & ~(COLUMN - 1));
}

static inline uint64_t get_bits_len(uint32_t room)
{
    return (pad_column(8ull * ((room + 63) / 64)));
}

static inline uint32_t get_bits(uint32_t column, uint32_t room)
{
    return (column - (uint32_t)COLUMN - (uint32_t)get_bits_len(room));
}

#endif
