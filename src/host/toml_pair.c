/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "toml.h"

static char **copy_key(char **key, uint32_t depth)
{
    char **copy = malloc(sizeof(*copy) * (depth + 1));

    if (copy)
        memcpy(copy, key, sizeof(*copy) * depth);
    return (copy);
}

static bool read_member(reading_t *r, char **key, uint32_t depth)
{
    char **whole = copy_key(key, depth);
    uint32_t at = depth;
    value_t v = {};

    toml_skip(r);
    if (!whole || !toml_read_key(r, &whole, &at))
        return (toml_refuse(r, "an inline table with no name in it"));
    toml_skip(r);
    if (*r->at != '=')
        return (toml_refuse(r, "a name with no = after it"));
    r->at++;
    toml_skip(r);
    if (*r->at == '{')
        return (toml_read_inline(r, whole, at));
    return (toml_read_value(r, &v) && toml_add(r, whole, at, &v));
}

bool toml_read_inline(reading_t *r, char **key, uint32_t depth)
{
    bool more = true;

    r->at++;
    while (more) {
        if (!read_member(r, key, depth))
            return (false);
        toml_skip(r);
        more = *r->at == ',';
        r->at += more;
    }
    if (*r->at != '}')
        return (toml_refuse(r, "an inline table with no } after it"));
    r->at++;
    return (true);
}

bool toml_read_pair(reading_t *r)
{
    char **key = nullptr;
    uint32_t depth = 0;
    value_t v = {};

    r->start = r->line;
    if (!toml_read_key(r, &key, &depth))
        return (false);
    toml_skip(r);
    if (*r->at != '=')
        return (toml_refuse(r, "a key with no = after it"));
    r->at++;
    toml_skip(r);
    if (*r->at == '{')
        return (toml_read_inline(r, key, depth));
    return (toml_read_value(r, &v) && toml_add(r, key, depth, &v));
}
