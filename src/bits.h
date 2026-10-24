/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_BITS_H
    #define BRIDGE_SRC_BITS_H

    #include <stddef.h>
    #include <stdint.h>

static inline uint32_t bits_words(uint32_t n)
{
    return ((uint32_t)(((uint64_t)n + 63) / 64));
}

static inline bool bits_has(const uint64_t *bits, uint32_t i)
{
    return (bits[i >> 6] >> (i & 63) & 1);
}

static inline void bits_set(uint64_t *bits, uint32_t i)
{
    bits[i >> 6] |= (uint64_t)1 << (i & 63);
}

static inline void bits_clear(uint64_t *bits, uint32_t i)
{
    bits[i >> 6] &= ~((uint64_t)1 << (i & 63));
}

static inline void bits_or(uint64_t *to, const uint64_t *from, uint32_t words)
{
    for (uint32_t w = 0; w < words; w++)
        to[w] |= from[w];
}

#endif
