/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "config.h"

void names_add(names_t *into, const char *name)
{
    const char **more = nullptr;

    for (uint32_t i = 0; i < into->count; i++)
        if (!strcmp(into->names[i], name))
            return;
    more = realloc(into->names, sizeof(*more) * (into->count + 1));
    if (!more)
        return;
    into->names = more;
    into->names[into->count++] = name;
}

static void add_names(const cfile_t *f, const keyed_t *under, names_t *into)
{
    const entry_t *e = nullptr;

    for (uint32_t i = 0; i < f->toml.count; i++) {
        e = &f->toml.entries[i];
        if (e->value.kind && e->depth > under->depth
            && toml_has_prefix(e, under->names, under->depth))
            names_add(into, e->key[under->depth]);
    }
}

names_t config_names(const config_t *c, const keyed_t *under)
{
    names_t names = {};

    for (uint32_t f = 0; f < c->count; f++)
        add_names(&c->files[f], under, &names);
    return (names);
}
