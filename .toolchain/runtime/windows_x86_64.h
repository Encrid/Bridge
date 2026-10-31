/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef WINDOWS_X86_64_H
    #define WINDOWS_X86_64_H

    #include <stddef.h>
    #include <stdint.h>

constexpr size_t SHORT_PATH = 512;

typedef uint64_t word8_t [[gnu::aligned(1), gnu::may_alias]];
typedef uint32_t word4_t [[gnu::aligned(1), gnu::may_alias]];
typedef uint16_t word2_t [[gnu::aligned(1), gnu::may_alias]];

void *memcpy(void *to, const void *from, size_t len);
void *memmove(void *to, const void *from, size_t len);
void *memset(void *to, int fill, size_t len);
int memcmp(const void *left, const void *right, size_t len);
size_t strlen(const char *text);
void ___chkstk_ms(void);

[[gnu::dllimport]] void move_memory(void *to, const void *from, size_t len)
    __asm__("RtlMoveMemory");
[[gnu::dllimport]] void fill_memory(void *to, size_t len, unsigned char fill)
    __asm__("RtlFillMemory");

#endif
