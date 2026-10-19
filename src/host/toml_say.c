/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "toml.h"

bool toml_is_same(const value_t *a, const value_t *b)
{
    if (a->kind != b->kind)
        return (false);
    if (a->kind == VALUE_TEXT)
        return (!strcmp(a->text, b->text));
    if (a->kind != VALUE_LIST)
        return (a->flag == b->flag && a->whole == b->whole
            && a->real == b->real);
    if (a->count != b->count)
        return (false);
    for (uint32_t i = 0; i < a->count; i++)
        if (!toml_is_same(&a->items[i], &b->items[i]))
            return (false);
    return (true);
}

static void say_scalar(const value_t *v, char *out, size_t size)
{
    if (v->kind == VALUE_TEXT)
        snprintf(out, size, "\"%s\"", v->text);
    if (v->kind == VALUE_WHOLE)
        snprintf(out, size, "%lld", (long long)v->whole);
    if (v->kind == VALUE_REAL)
        snprintf(out, size, "%g", v->real);
    if (v->kind == VALUE_FLAG)
        snprintf(out, size, "%s", v->flag ? "true" : "false");
}

void toml_say(const value_t *v, char *out, size_t size)
{
    size_t used = 0;

    if (v->kind != VALUE_LIST) {
        say_scalar(v, out, size);
        return;
    }
    used += (size_t)snprintf(out, size, "[");
    for (uint32_t i = 0; i < v->count && used + 2 < size; i++) {
        used += (size_t)snprintf(out + used, size - used, i ? ", " : "");
        toml_say(&v->items[i], out + used, size - used);
        used += strlen(out + used);
    }
    if (used + 1 < size)
        snprintf(out + used, size - used, "]");
}
