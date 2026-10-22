/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "manifest_inside.h"

static size_t read_leb(const unsigned char *bytes, size_t len, size_t *at)
{
    size_t n = 0;
    int shift = 0;

    while (*at < len && shift < 35) {
        n |= (size_t)(bytes[*at] & 0x7f) << shift;
        shift += 7;
        if (!(bytes[(*at)++] & 0x80))
            return (n);
    }
    return (len);
}

static bool is_manifest(const unsigned char *bytes, size_t end, size_t *at)
{
    static const char name[] = "bridge:manifest";
    size_t named = read_leb(bytes, end, at);

    if (named != sizeof(name) - 1 || named > end - *at
        || memcmp(bytes + *at, name, named))
        return (false);
    *at += named;
    return (true);
}

bool manifest_find_wasm(const unsigned char *bytes, size_t len, size_t *at,
    size_t *size)
{
    size_t next = 8;
    size_t end;
    bool custom;

    while (next < len) {
        custom = !bytes[next];
        *at = next + 1;
        end = read_leb(bytes, len, at);
        if (end > len - *at)
            return (false);
        end += *at;
        next = end;
        if (custom && is_manifest(bytes, end, at)) {
            *size = end - *at;
            return (true);
        }
    }
    return (false);
}
