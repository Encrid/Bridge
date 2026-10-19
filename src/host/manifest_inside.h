/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_HOST_MANIFEST_INSIDE_H
    #define BRIDGE_HOST_MANIFEST_INSIDE_H

    #include "manifest.h"

typedef bool (*finder_t)(const unsigned char *bytes, size_t len, size_t *at,
    size_t *size);

typedef struct {
    const char *first;
    size_t count;
    finder_t find;
} format_t;

[[nodiscard]] uint64_t manifest_number(const unsigned char *bytes, size_t len,
    size_t at, int width);

[[nodiscard]] bool manifest_find_elf(const unsigned char *bytes, size_t len,
    size_t *at, size_t *size);

[[nodiscard]] bool manifest_find_macho(const unsigned char *bytes,
    size_t len, size_t *at, size_t *size);

[[nodiscard]] bool manifest_find_wasm(const unsigned char *bytes, size_t len,
    size_t *at, size_t *size);

[[nodiscard]] const char *manifest_find(const unsigned char *bytes, size_t len,
    size_t *at, size_t *size);

[[nodiscard]] int manifest_take(manifest_t *m, const char *key,
    const char *value);

#endif
