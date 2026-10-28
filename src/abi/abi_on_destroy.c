/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "abi_declare.h"

static const char *const ON_DESTROY_NAMES[] = {
    "",
    "BRIDGE_ON_DESTROY_REMOVE",
    "BRIDGE_ON_DESTROY_KEEP"
};

int abi_check_on_destroy(const declaring_t *g)
{
    const bridge_declaration_t *d = &g->d;

    if (d->on_destroy < BRIDGE_ON_DESTROY_REMOVE
        || d->on_destroy > BRIDGE_ON_DESTROY_KEEP)
        return (abi_refuse(g->m, "type '%s': on_destroy is unset", g->name));
    if (d->storage == BRIDGE_SINGLETON
        && d->on_destroy != BRIDGE_ON_DESTROY_REMOVE)
        return (abi_refuse(g->m, "type '%s' is a singleton, and a singleton "
            "states BRIDGE_ON_DESTROY_REMOVE", g->name));
    return (0);
}

int abi_check_same_on_destroy(const declaring_t *g)
{
    if (g->d.on_destroy == g->c->on_destroy)
        return (0);
    return (abi_refuse(g->m, "type '%s' states on_destroy %s here, and it "
        "was declared with %s", g->name, ON_DESTROY_NAMES[g->d.on_destroy],
        ON_DESTROY_NAMES[g->c->on_destroy]));
}
