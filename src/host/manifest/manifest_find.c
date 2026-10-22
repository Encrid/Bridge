/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "manifest_inside.h"

constexpr size_t FORMATS_COUNT = 3;

uint64_t manifest_number(const unsigned char *bytes, size_t len, size_t at,
    int width)
{
    uint64_t n = 0;

    if (at > len || (size_t)width > len - at)
        return (0);
    while (width--)
        n = n << 8 | bytes[at + (size_t)width];
    return (n);
}

static bool find_library(const unsigned char *bytes, size_t len, size_t *at,
    size_t *size)
{
    size_t pe = manifest_number(bytes, len, 0x3c, 4);
    size_t count = manifest_number(bytes, len, pe + 6, 2);
    size_t table = pe + 24 + manifest_number(bytes, len, pe + 20, 2);
    size_t i = 0;

    if (pe + 24 > len || memcmp(bytes + pe, "PE\0", 4))
        return (false);
    while (i < count && table + 40 <= len) {
        *at = manifest_number(bytes, len, table + 20, 4);
        *size = manifest_number(bytes, len, table + 8, 4);
        if (*size > manifest_number(bytes, len, table + 16, 4))
            *size = manifest_number(bytes, len, table + 16, 4);
        if (!memcmp(bytes + table, ".bridge", 8))
            return (*at <= len && *size <= len - *at);
        table += 40;
        i++;
    }
    return (false);
}

static const format_t FORMATS[FORMATS_COUNT] = { { "MZ", 2, find_library },
    { "\x7f" "ELF\x02\x01", 6, manifest_find_elf },
    { "\xcf\xfa\xed\xfe", 4, manifest_find_macho } };

const char *manifest_find(const unsigned char *bytes, size_t len, size_t *at,
    size_t *size)
{
    bool wasm = len >= 8 && !memcmp(bytes, "\0asm\1\0\0", 8);
    size_t i = 0;

    while (!wasm && i < FORMATS_COUNT && (len < 0x40
        || memcmp(bytes, FORMATS[i].first, FORMATS[i].count)))
        i++;
    if (!wasm && i == FORMATS_COUNT)
        return (nullptr);
    if (!(wasm ? manifest_find_wasm : FORMATS[i].find)(bytes, len, at, size))
        *size = 0;
    return (wasm ? "wasm" : "native");
}
