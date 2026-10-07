/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/module_types.c -- components leaving: a module stops declaring what
** it declared.
**
** lifetime.md, *What unload leaves behind*
*/

#include <stdlib.h>

#include "module.h"

static void unlist_type(bridge_t *b, const declared_type_t *c)
{
    uint32_t k = 0;

    while (b->types[k] != c)
        k++;
    b->type_count--;
    b->types[k] = b->types[b->type_count];
}

/*
** delete_type
**
** A component's record goes: what storage knows of it, and its hold on the
** types its fields name, which go with it when nothing else declares or
** names them.
*/
static void delete_type(bridge_t *b, declared_type_t *c)
{
    declared_type_t *named;
    uint32_t k = 0;

    storage_undeclare(b->storage, c->type);
    unlist_type(b, c);
    while (k < c->dep_count) {
        named = c->deps[k];
        named->users--;
        if (!named->users && !named->declarers)
            delete_type(b, named);
        k++;
    }
    schema_free(&c->s);
    free(c->runs);
    free(c->deps);
    free(c->name);
    free(c->zero);
    free(c);
}

/*
** release_type
**
** A module stops declaring a type. Shared, it stays; the last to leave
** applies its own policy. Policy zero is a load that failed. A type some
** other component's field still names is kept as a type, with no rows.
*/
static void release_type(bridge_t *b, declared_type_t *c,
    enum bridge_on_unload policy)
{
    c->declarers--;
    if (c->declarers)
        return;
    if (policy)
        storage_drop(b->storage, c->type, policy);
    if (!c->users)
        delete_type(b, c);
    b->layout_moved = 1;
}

/*
** module_release_types
**
** A module stops declaring what it declared, last declared first: a type
** is used by what was declared after it.
*/
void module_release_types(bridge_t *b, module_t *m, bool by_policy)
{
    enum bridge_on_unload policy = 0;
    uint32_t i = m->declared_count;

    while (i) {
        i--;
        if (by_policy)
            policy = m->policy[i];
        release_type(b, m->declared[i], policy);
    }
    m->declared_count = 0;
}
