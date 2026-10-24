/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_SRC_HASH_H
    #define BRIDGE_SRC_HASH_H

    #include <stddef.h>
    #include <stdint.h>

static inline uint64_t hash_fnv1a(const void *bytes, size_t len)
{
    const uint8_t *at = bytes;
    uint64_t h = 1'469'598'103'934'665'603ull;
    size_t i = 0;

    while (i < len) {
        h = (h ^ at[i]) * 1'099'511'628'211ull;
        i++;
    }
    return (h);
}

#endif
