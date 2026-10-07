/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/abi_names.c -- names: whose a type's is, where a bare one is looked
** for, and who else declares one.
*/

#include <stdio.h>
#include <string.h>

#include "abi_declare.h"

/*
** abi_is_listed
**
** Answers whether a list of names holds a name.
*/
bool abi_is_listed(char **list, uint32_t count, const char *name)
{
    uint32_t i = 0;

    while (i < count && strcmp(list[i], name))
        i++;
    return (i < count);
}

/*
** abi_is_foreign
**
** Answers whether a name is of another namespace than the module's.
*/
bool abi_is_foreign(const module_t *m, const char *name)
{
    size_t n = strlen(m->ns);

    return (strncmp(name, m->ns, n) || name[n] != ':');
}

/*
** abi_find_type
**
** Finds the type a field names bare, in the namespace of the type being
** declared. A tag has no bytes for a field to be, and a type a reload has
** made anew is the one that is meant.
*/
bool abi_find_type(void *ctx, const char *bare, schema_named_t *named)
{
    const declaring_t *g = ctx;
    const bridge_t *b = g->m->b;
    size_t ns = strlen(g->name);
    declared_type_t *c = nullptr;
    uint32_t i = 0;

    while (i < b->type_count && !c) {
        c = b->types[i];
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

/*
** declares_it
**
** Answers whether a module other than the one declaring, and than the one
** it reloads, declares a type.
*/
static bool declares_it(const module_t *o, const module_t *m,
    const declared_type_t *c)
{
    uint32_t i = 0;

    if (o == m || o == m->reloads || !strcmp(o->identity, m->identity))
        return (false);
    while (i < o->declared_count && o->declared[i] != c)
        i++;
    return (i < o->declared_count);
}

/*
** abi_say_others
**
** Writes who else declares a type, as the end of a refusal.
*/
void abi_say_others(const module_t *m, const declared_type_t *c,
    abi_others_t *others)
{
    const module_t *o = m->b->modules;
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
