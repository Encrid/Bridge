/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <emmintrin.h>

#include "windows_x86_64.h"

[[clang::no_builtin, gnu::always_inline]]
static void fill_small(unsigned char *to, uint64_t word, size_t len)
{
    if (len >= 4) {
        *(word4_t *)to = (uint32_t)word;
        *(word4_t *)(to + len - 4) = (uint32_t)word;
    } else if (len >= 2) {
        *(word2_t *)to = (uint16_t)word;
        *(word2_t *)(to + len - 2) = (uint16_t)word;
    } else if (len) {
        *to = (unsigned char)word;
    }
}

[[clang::no_builtin, gnu::always_inline]]
static void fill_words(unsigned char *to, unsigned char byte, size_t len)
{
    uint64_t word = 0x0101010101010101ull * byte;

    if (len >= 16) {
        *(word8_t *)to = word;
        *(word8_t *)(to + 8) = word;
        *(word8_t *)(to + len - 16) = word;
        *(word8_t *)(to + len - 8) = word;
    } else if (len >= 8) {
        *(word8_t *)to = word;
        *(word8_t *)(to + len - 8) = word;
    } else {
        fill_small(to, word, len);
    }
}

[[clang::no_builtin, gnu::always_inline]]
static void fill_vector(unsigned char *to, unsigned char byte, size_t len)
{
    __m128i v = _mm_set1_epi8((char)byte);

    if (len <= 64) {
        _mm_storeu_si128((__m128i *)to, v);
        _mm_storeu_si128((__m128i *)(to + 16), v);
        _mm_storeu_si128((__m128i *)(to + len - 32), v);
    } else {
        for (size_t i = 0; i < len - 16; i += 16)
            _mm_storeu_si128((__m128i *)(to + i), v);
    }
    _mm_storeu_si128((__m128i *)(to + len - 16), v);
}

[[clang::no_builtin]]
void *memset(void *to, int fill, size_t len)
{
    unsigned char byte = (unsigned char)fill;

    if (len <= 32)
        fill_words(to, byte, len);
    else if (len < SHORT_PATH)
        fill_vector(to, byte, len);
    else
        fill_memory(to, len, byte);
    return (to);
}
