/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "abi_declare.h"

bool abi_is_listed(char **list, uint32_t count, const char *name)
{
    uint32_t i = 0;

    while (i < count && strcmp(list[i], name))
        i++;
    return (i < count);
}

bool abi_is_foreign(const module_t *m, const char *name)
{
    size_t n = strlen(m->ns);

    return (strncmp(name, m->ns, n) || name[n] != ':');
}

bool abi_find_type(void *ctx, const char *bare, schema_named_t *named)
{
    const declaring_t *g = ctx;
    const bridge_world_t *w = g->m->w;
    size_t ns = strlen(g->name);
    declared_type_t *c = nullptr;
    uint32_t i = 0;

    while (i < w->type_count && !c) {
        c = w->types[i];
        if (strncmp(c->name, g->name, ns) || c->name[ns] != ':'
            || strcmp(c->name + ns + 1, bare) || c->kind == BRIDGE_TAG
            || c->superseded)
            c = nullptr;
        i++;
    }
    if (c)
        *named = (schema_named_t){ c->kind, c->size, c->align, c };
    return (c != nullptr);
}

static bool declares_it(const module_t *o, const module_t *m,
    const declared_type_t *c)
{
    uint32_t i = 0;

    if (o == m->reloads)
        return (false);
    while (i < o->declared_count && o->declared[i] != c)
        i++;
    return (i < o->declared_count);
}

void abi_say_others(const module_t *m, const declared_type_t *c,
    abi_others_t *others)
{
    const module_t *o = m->w->modules;
    size_t size = sizeof(others->text);
    size_t used = 0;

    others->text[0] = 0;
    while (o) {
        if (used < size && declares_it(o, m, c))
            used += (size_t)snprintf(others->text + used, size - used,
                "%s'%s'", used ? ", " : ". It is also declared by ",
                o->identity);
        o = o->next;
    }
}
