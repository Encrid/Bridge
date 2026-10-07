/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/storage_bytes.h -- a value copied or made zero, and where a column
** and a toggled column's bits begin in a block.
*/

#ifndef BRIDGE_SRC_STORAGE_BYTES_H
    #define BRIDGE_SRC_STORAGE_BYTES_H

    #include <string.h>

    #include "storage_inside.h"

/*
** copy_bytes
**
** Copies a value. A value is a word or two more often than not, and a copy
** of a length the compiler cannot see is a call: the two commonest sizes are
** done here, for every row that moves.
*/
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

/*
** get_bits_len
**
** Answers how many bytes the bits of a toggled column of `room` rows take.
** A toggled column has a bit a row, a 64-bit word for each 64 rows, ahead of
** it in the block; and between the two, sixteen bytes that say where the
** bits begin, which is where a module that has the column finds them.
**
** storage.md, *A toggle*
*/
static inline uint64_t get_bits_len(uint32_t room)
{
    return (pad_column(8ull * ((room + 63) / 64)));
}

/*
** get_bits
**
** Answers where the bits of the toggled column at `column` are, in a block
** of `room` rows.
*/
static inline uint32_t get_bits(uint32_t column, uint32_t room)
{
    return (column - (uint32_t)COLUMN - (uint32_t)get_bits_len(room));
}

#endif
