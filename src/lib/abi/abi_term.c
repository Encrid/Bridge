/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "abi_subscribe.h"

static int check_values(subscribing_t *g, const bridge_term_t *t)
{
    if (!t->access)
        return (abi_refuse(g->m, "%s: its access is unset", g->of));
    if (t->access > BRIDGE_WRITE)
        return (abi_refuse(g->m, "%s: its access is %u, and there is read "
            "and write", g->of, t->access));
    if (!t->oper)
        return (abi_refuse(g->m, "%s: its operator is unset", g->of));
    if (t->oper > BRIDGE_OPTIONAL)
        return (abi_refuse(g->m, "%s: its operator is %u, and there are "
            "three", g->of, t->oper));
    return (0);
}

static bool selects_alone(const subscribing_t *g, uint32_t i)
{
    bridge_term_t other;
    uint32_t k = 0;

    while (k < g->count) {
        memcpy(&other, &g->terms[k], sizeof(other));
        if (k != i && other.oper != BRIDGE_OPTIONAL)
            return (false);
        k++;
    }
    return (true);
}

static int check_sparse(subscribing_t *g, uint32_t i, const bridge_term_t *t,
    const declared_type_t *c)
{
    if (!selects_alone(g, i))
        return (abi_refuse(g->m, "%s: '%s' is kept in a sparse set, and "
            "does not select the rows of a table. It is stated optional and "
            "looked for at each row, or it is the one term that selects",
            g->of, c->name));
    if (t->oper == BRIDGE_NOT)
        return (abi_refuse(g->m, "%s: '%s' is kept in a sparse set, and a "
            "system cannot walk the entities that are not in it", g->of,
            c->name));
    return (0);
}

static int check_walked(subscribing_t *g, uint32_t i, const bridge_term_t *t,
    const declared_type_t *c)
{
    if (g->events || t->oper == BRIDGE_OPTIONAL)
        return (0);
    if (c->storage == BRIDGE_SINGLETON)
        return (abi_refuse(g->m, "%s: '%s' is a singleton, and is on no "
            "entity a system walks. It is stated optional and read where it "
            "is", g->of, c->name));
    if (c->storage == BRIDGE_DIRECT_INDEXED)
        return (abi_refuse(g->m, "%s: '%s' is direct-indexed, and does not "
            "select what a system walks. It is stated optional and reached "
            "by the entity's id", g->of, c->name));
    if (c->storage == BRIDGE_SPARSE_SET)
        return (check_sparse(g, i, t, c));
    return (0);
}

int abi_check_term(subscribing_t *g, uint32_t i, const bridge_term_t *t)
{
    module_t *m = g->m;
    const declared_type_t *c;

    if (t->slot >= m->declared_count)
        return (abi_refuse(m, "%s names slot %u, which it never declared",
            g->of, t->slot));
    c = m->declared[t->slot];
    if (check_values(g, t))
        return (1);
    if (t->access == BRIDGE_WRITE && !abi_is_writable(m, c))
        return (abi_refuse(m, "%s '%s' may not write '%s': no grant says so",
            g->what, g->name, c->name));
    if (m->refused)
        return (1);
    return (check_walked(g, i, t, c));
}
