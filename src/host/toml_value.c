/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "toml.h"

static bool read_flag(reading_t *r, value_t *v)
{
    bool yes = !strncmp(r->at, "true", 4);

    if (!yes && strncmp(r->at, "false", 5))
        return (toml_refuse(r, "a value that is none"));
    *v = (value_t){ .kind = VALUE_FLAG, .flag = yes };
    r->at += yes ? 4 : 5;
    return (true);
}

static void skip_space(reading_t *r)
{
    toml_skip(r);
    while (*r->at == '#' || *r->at == '\n' || *r->at == '\r') {
        if (*r->at == '#')
            r->at += strcspn(r->at, "\n");
        else
            r->line += *r->at++ == '\n';
        toml_skip(r);
    }
}

static bool add_item(reading_t *r, value_t *list, const value_t *item)
{
    value_t *more = realloc(list->items, sizeof(*more) * (list->count + 1));

    if (!more)
        return (toml_refuse(r, "no memory left"));
    list->items = more;
    list->items[list->count++] = *item;
    return (true);
}

static bool read_list(reading_t *r, value_t *v)
{
    value_t item = {};

    *v = (value_t){ .kind = VALUE_LIST };
    r->at++;
    skip_space(r);
    while (*r->at != ']') {
        if (!toml_read_value(r, &item) || !add_item(r, v, &item))
            return (false);
        skip_space(r);
        if (*r->at != ',' && *r->at != ']')
            return (toml_refuse(r, "a list with no , or ] after a value"));
        r->at += *r->at == ',';
        skip_space(r);
    }
    r->at++;
    return (true);
}

bool toml_read_value(reading_t *r, value_t *v)
{
    char c = *r->at;

    if (c == '"' || c == '\'') {
        *v = (value_t){ .kind = VALUE_TEXT, .text = toml_read_text(r) };
        return (v->text != nullptr);
    }
    if (c == '[')
        return (read_list(r, v));
    if (c == 't' || c == 'f')
        return (read_flag(r, v));
    if (c == '{')
        return (toml_refuse(r, "an inline table in a list, not taken"));
    if (c == '+' || c == '-' || (c >= '0' && c <= '9'))
        return (toml_read_number(r, v));
    return (toml_refuse(r, "a value that is none"));
}
