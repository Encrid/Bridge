/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "manifest_inside.h"

static bool named(const unsigned char *bytes, size_t len, size_t at,
    const char *name)
{
    size_t size = strlen(name) + 1;

    return (at <= len && size <= len - at && !memcmp(bytes + at, name, size));
}

bool manifest_find_elf(const unsigned char *bytes, size_t len, size_t *at,
    size_t *size)
{
    size_t table = manifest_number(bytes, len, 0x28, 8);
    size_t count = manifest_number(bytes, len, 0x3c, 2);
    size_t names = manifest_number(bytes, len, 0x3e, 2);
    size_t entry;

    if (table > len || count > (len - table) / 64 || names >= count)
        return (false);
    names = manifest_number(bytes, len, table + 64 * names + 24, 8);
    for (size_t i = 0; i < count && names <= len; i++) {
        entry = table + 64 * i;
        *at = manifest_number(bytes, len, entry + 24, 8);
        *size = manifest_number(bytes, len, entry + 32, 8);
        if (named(bytes, len, names + manifest_number(bytes, len, entry, 4),
            ".bridge"))
            return (*at <= len && *size <= len - *at);
    }
    return (false);
}
