/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/abi_replace.c -- a declaration of a name there already is a type of:
** it is that type, or, at a reload of the one module that declares it, the
** type that takes its place.
**
** lifetime.md, *When a component layout changed*
*/

#include <stdlib.h>
#include <string.h>

#include "abi_declare.h"

/*
** find_named
**
** Answers the type of a name there is, that no reload has made anew, or
** nullptr.
*/
static declared_type_t *find_named(const bridge_t *b, const char *name)
{
    declared_type_t *c = nullptr;
    uint32_t i = 0;

    while (i < b->type_count) {
        if (!strcmp(b->types[i]->name, name) && !b->types[i]->superseded)
            c = b->types[i];
        i++;
    }
    return (c);
}

/*
** is_reloaded
**
** Answers whether a module is a reload of one that declares a type.
*/
static bool is_reloaded(const module_t *m, const declared_type_t *c)
{
    const module_t *old = m->reloads;
    uint32_t i = 0;

    if (!old)
        return (false);
    while (i < old->declared_count && old->declared[i] != c)
        i++;
    return (i < old->declared_count);
}

/*
** replace_changed
**
** Refuses a type that has another component layout than it was declared
** with, unless it is exclusive to the module reloading and no other
** module's rows hold a value of it. Shared, it is refused, naming who else
** declares it.
**
** lifetime.md, *When a component layout changed*
*/
static int replace_changed(declaring_t *g)
{
    module_t *m = g->m;
    const declared_type_t *c = g->c;
    const declared_type_t *holder;
    const module_t *by = nullptr;
    abi_others_t others;

    abi_say_others(m, c, &others);
    if (others.text[0] || !is_reloaded(m, c))
        return (abi_refuse(m, "type '%s' has another component layout here: "
            "fingerprint %016llx, and it was declared with %016llx%s", g->name,
            (unsigned long long)g->print, (unsigned long long)c->fingerprint,
            others.text));
    holder = abi_find_holder(m, c, &by);
    if (holder)
        return (abi_refuse(m, "type '%s' has another component layout here, "
            "and '%s', which module '%s' declares, has a field of it", g->name,
            holder->name, by->identity));
    return (c->storage != g->d.storage && abi_check_move(m, c, g->d.storage));
}

/*
** replace_moved
**
** Refuses a type stated in another storage layout than it was declared in,
** unless it is exclusive to the module reloading. A module reads a component
** by code compiled for one storage layout, so every declarer states the
** same.
**
** storage.md, *Storage layouts*
*/
static int replace_moved(declaring_t *g)
{
    module_t *m = g->m;
    const declared_type_t *c = g->c;
    abi_others_t others;

    abi_say_others(m, c, &others);
    if (others.text[0] || !is_reloaded(m, c))
        return (abi_refuse(m, "type '%s' is stated as %s here, and it was "
            "declared as %s%s", g->name, ABI_STORAGE_NAMES[g->d.storage],
            ABI_STORAGE_NAMES[c->storage], others.text));
    return (abi_check_move(m, c, g->d.storage));
}

/*
** abi_replace_type
**
** Decides what becomes of the type of that name there already is. One that
** this declaration replaces takes the rows if the reload holds; whatever an
** earlier reload that failed worked out for it is not this one's.
*/
int abi_replace_type(declaring_t *g)
{
    declared_type_t *c = find_named(g->m->b, g->name);
    bool changed = c && c->fingerprint != g->print;

    g->c = c;
    changed = changed || (c && abi_names_superseded(c));
    if (!c || (!changed && c->storage == g->d.storage))
        return (0);
    if (changed ? replace_changed(g) : replace_moved(g))
        return (1);
    g->was = c;
    g->c = nullptr;
    free(c->runs);
    c->runs = nullptr;
    c->run_count = 0;
    return (0);
}
