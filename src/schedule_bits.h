/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_SCHEDULE_BITS_H
    #define BRIDGE_SRC_SCHEDULE_BITS_H

    #include <stddef.h>
    #include <stdint.h>

static inline uint32_t schedule_get_words(uint32_t n)
{
    return ((uint32_t)(((uint64_t)n + 63) / 64));
}

static inline size_t schedule_get_matrix_words(uint32_t n)
{
    return ((size_t)n * schedule_get_words(n) + 1);
}

static inline bool schedule_has_bit(const uint64_t *bits, uint32_t j)
{
    return (bits[j / 64] >> (j % 64) & 1);
}

static inline void schedule_put_bit(uint64_t *bits, uint32_t j)
{
    bits[j / 64] |= (uint64_t)1 << (j % 64);
}

static inline void schedule_join_bits(uint64_t *to, const uint64_t *from,
    uint32_t words)
{
    for (uint32_t w = 0; w < words; w++)
        to[w] |= from[w];
}

#endif
