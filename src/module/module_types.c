/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "module.h"

static void unlist_type(bridge_world_t *w, const declared_type_t *c)
{
    uint32_t k = 0;

    while (w->types[k] != c)
        k++;
    w->type_count--;
    w->types[k] = w->types[w->type_count];
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

static void delete_type(bridge_world_t *w, declared_type_t *c)
{
    declared_type_t *named;
    uint32_t k = 0;

    storage_undeclare(w->storage, c->type);
    unlist_type(w, c);
    while (k < c->dep_count) {
        named = c->deps[k];
        named->users--;
        if (!named->users && !named->declarers)
            delete_type(w, named);
        k++;
    }
    module_free_type(c);
}

static void release_type(bridge_world_t *w, declared_type_t *c,
    enum bridge_on_unload policy)
{
    c->declarers--;
    if (c->declarers)
        return;
    if (policy)
        storage_drop(w->storage, c->type, policy);
    if (!c->users)
        delete_type(w, c);
    w->layout_moved = 1;
}

void module_release_types(bridge_world_t *w, module_t *m, bool by_policy)
{
    enum bridge_on_unload policy = 0;
    uint32_t i = 0;

    while (i < m->declared_count) {
        if (m->declared[i]->declarers == 1)
            storage_let_keep(w->storage, m->declared[i]->type, false);
        i++;
    }
    while (i) {
        i--;
        if (by_policy)
            policy = m->policy[i];
        release_type(w, m->declared[i], policy);
    }
    m->declared_count = 0;
}
