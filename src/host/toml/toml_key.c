/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "toml.h"

static bool is_bare(char c)
{
    return ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
        || (c >= '0' && c <= '9') || c == '_' || c == '-');
}

static char *read_name(reading_t *r)
{
    size_t len = 0;
    char *name = nullptr;

    if (*r->at == '"' || *r->at == '\'')
        return (toml_read_text(r));
    while (is_bare(r->at[len]))
        len++;
    if (len)
        name = malloc(len + 1);
    if (!name) {
        toml_refuse(r, "a key with no name");
        return (nullptr);
    }
    memcpy(name, r->at, len);
    name[len] = 0;
    r->at += len;
    return (name);
}

bool toml_read_key(reading_t *r, char ***key, uint32_t *depth)
{
    char *name = read_name(r);
    char **more = nullptr;

    while (name) {
        more = realloc(*key, sizeof(*more) * (*depth + 1));
        if (!more)
            return (toml_refuse(r, "no memory left"));
        *key = more;
        (*key)[(*depth)++] = name;
        toml_skip(r);
        if (*r->at != '.')
            return (true);
        r->at++;
        toml_skip(r);
        name = read_name(r);
    }
    return (false);
}
