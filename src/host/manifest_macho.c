/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "manifest_inside.h"

static size_t in_segment(const unsigned char *bytes, size_t len,
    size_t command)
{
    size_t count = manifest_number(bytes, len, command + 64, 4);
    size_t section = command + 72;

    for (size_t i = 0; i < count && section <= len && len - section >= 80;
        i++, section += 80)
        if (!memcmp(bytes + section, ".bridge\0", 8)
            && !memcmp(bytes + section + 16, "__TEXT\0", 7))
            return (section);
    return (0);
}

bool manifest_find_macho(const unsigned char *bytes, size_t len, size_t *at,
    size_t *size)
{
    size_t count = manifest_number(bytes, len, 16, 4);
    size_t command = 32;
    size_t section = 0;
    size_t step;

    for (size_t i = 0; i < count && command < len && !section; i++) {
        if (manifest_number(bytes, len, command, 4) == 0x19)
            section = in_segment(bytes, len, command);
        step = manifest_number(bytes, len, command + 4, 4);
        command = step ? command + step : len;
    }
    if (!section)
        return (false);
    *size = manifest_number(bytes, len, section + 40, 8);
    *at = manifest_number(bytes, len, section + 48, 4);
    return (*at <= len && *size <= len - *at);
}
