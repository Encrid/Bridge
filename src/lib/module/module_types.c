/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
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

void module_free_type(declared_type_t *c)
{
    if (!c)
        return;
    schema_free(&c->s);
    free(c->runs);
    free(c->deps);
    free(c->name);
    free(c->zero);
    free(c->desc);
    free(c);
}

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
    module_free_type(c);
}

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
