/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <emmintrin.h>

#include "windows_x86_64.h"

[[clang::no_builtin, gnu::always_inline]]
static void move_tiny(unsigned char *to, const unsigned char *from, size_t len)
{
    uint16_t head;
    uint16_t tail;

    if (len >= 2) {
        head = *(const word2_t *)from;
        tail = *(const word2_t *)(from + len - 2);
        *(word2_t *)to = head;
        *(word2_t *)(to + len - 2) = tail;
    } else if (len) {
        *to = *from;
    }
}

[[clang::no_builtin, gnu::always_inline]]
static void move_small(unsigned char *to, const unsigned char *from,
    size_t len)
{
    uint64_t head;
    uint64_t tail;

    if (len >= 8) {
        head = *(const word8_t *)from;
        tail = *(const word8_t *)(from + len - 8);
        *(word8_t *)to = head;
        *(word8_t *)(to + len - 8) = tail;
    } else if (len >= 4) {
        head = *(const word4_t *)from;
        tail = *(const word4_t *)(from + len - 4);
        *(word4_t *)to = (uint32_t)head;
        *(word4_t *)(to + len - 4) = (uint32_t)tail;
    } else {
        move_tiny(to, from, len);
    }
}

[[clang::no_builtin, gnu::always_inline]]
static void move_vector(unsigned char *to, const unsigned char *from,
    size_t len)
{
    __m128i head = _mm_loadu_si128((const __m128i *)from);
    __m128i tail = _mm_loadu_si128((const __m128i *)(from + len - 16));
    __m128i second;
    __m128i third;

    if (len > 64) {
        for (size_t i = 16; i < len - 16; i += 16)
            _mm_storeu_si128((__m128i *)(to + i),
                _mm_loadu_si128((const __m128i *)(from + i)));
    } else if (len > 32) {
        second = _mm_loadu_si128((const __m128i *)(from + 16));
        third = _mm_loadu_si128((const __m128i *)(from + len - 32));
        _mm_storeu_si128((__m128i *)(to + 16), second);
        _mm_storeu_si128((__m128i *)(to + len - 32), third);
    }
    _mm_storeu_si128((__m128i *)to, head);
    _mm_storeu_si128((__m128i *)(to + len - 16), tail);
}

[[clang::no_builtin]]
void *memcpy(void *to, const void *from, size_t len)
{
    if (len < 16)
        move_small(to, from, len);
    else if (len < SHORT_PATH)
        move_vector(to, from, len);
    else
        move_memory(to, from, len);
    return (to);
}

[[clang::no_builtin]]
void *memmove(void *to, const void *from, size_t len)
{
    if (len < 16)
        move_small(to, from, len);
    else if (len <= 64)
        move_vector(to, from, len);
    else
        move_memory(to, from, len);
    return (to);
}
