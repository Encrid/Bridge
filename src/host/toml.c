/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "toml.h"

bool toml_refuse(reading_t *r, const char *why)
{
    if (!r->into->why) {
        r->into->why = why;
        r->into->line = r->line;
    }
    return (false);
}

void toml_skip(reading_t *r)
{
    while (*r->at == ' ' || *r->at == '\t')
        r->at++;
}

static bool end_line(reading_t *r)
{
    toml_skip(r);
    if (*r->at == '#')
        r->at += strcspn(r->at, "\n");
    if (*r->at == '\r' && r->at[1] == '\n')
        r->at++;
    if (*r->at && *r->at != '\n')
        return (toml_refuse(r, "something more on its line"));
    if (*r->at)
        r->at++;
    r->line++;
    return (true);
}

static bool read_header(reading_t *r)
{
    char **name = nullptr;
    uint32_t depth = 0;

    r->start = r->line;
    r->at++;
    if (*r->at == '[')
        return (toml_refuse(r, "an array of tables, which is not taken"));
    toml_skip(r);
    if (!toml_read_key(r, &name, &depth))
        return (false);
    toml_skip(r);
    if (*r->at != ']')
        return (toml_refuse(r, "the name of a table with no ] after it"));
    r->at++;
    r->table_depth = 0;
    if (!toml_add(r, name, depth, &(value_t){}))
        return (false);
    r->table = name;
    r->table_depth = depth;
    return (true);
}

bool toml_read(const char *text, toml_t *t)
{
    reading_t r = { .at = text, .line = 1, .into = t };
    bool taken = true;

    *t = (toml_t){};
    if (!strncmp(r.at, "\xEF\xBB\xBF", 3))
        r.at += 3;
    while (taken && *r.at) {
        toml_skip(&r);
        if (*r.at == '[')
            taken = read_header(&r);
        else if (*r.at && !strchr("#\r\n", *r.at))
            taken = toml_read_pair(&r);
        taken = taken && end_line(&r);
    }
    return (taken);
}
