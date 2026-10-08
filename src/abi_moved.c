/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/abi_moved.c -- what a reload may not change of a type: one somebody
** else's rows hold, and a storage layout it cannot be moved to.
*/

#include "abi_declare.h"

/*
** abi_names_superseded
**
** Answers whether a field of a type names a type this reload has made anew.
*/
bool abi_names_superseded(const declared_type_t *c)
{
    uint32_t k = 0;

    while (k < c->dep_count && !c->deps[k]->superseded)
        k++;
    return (k < c->dep_count);
}

static bool has_field_of(const declared_type_t *other,
    const declared_type_t *c)
{
    uint32_t k = 0;

    while (k < other->dep_count && other->deps[k] != c)
        k++;
    return (k < other->dep_count);
}

/*
** find_declarer
**
** Answers a loaded module, other than the one reloading and the one it
** reloads, that declares a type, or nullptr.
*/
static const module_t *find_declarer(const module_t *m,
    const declared_type_t *type)
{
    const module_t *o = m->b->modules;
    uint32_t d;

    while (o) {
        d = 0;
        while (o != m && o != m->reloads && d < o->declared_count
            && o->declared[d] != type)
            d++;
        if (o != m && o != m->reloads && d < o->declared_count)
            return (o);
        o = o->next;
    }
    return (nullptr);
}

/*
** abi_find_holder
**
** Answers a type with a field of type `c` that another loaded module
** declares, and writes that module.
*/
const declared_type_t *abi_find_holder(const module_t *m,
    const declared_type_t *c, const module_t **by)
{
    const bridge_t *b = m->b;
    uint32_t i = 0;

    while (i < b->type_count) {
        *by = has_field_of(b->types[i], c) ? find_declarer(m, b->types[i])
            : nullptr;
        if (*by)
            return (b->types[i]);
        i++;
    }
    return (nullptr);
}

/*
** abi_check_move
**
** Refuses a move of a type to another storage layout that cannot be made:
** from a singleton, since there is no entity to carry it; to a singleton
** from more than one entity, since nothing says which has its value; and of
** a toggle that is off for some rows, unless the reload accepts that they
** are then carried like the rest.
**
** lifetime.md, *When a component is moved*
*/
int abi_check_move(module_t *m, const declared_type_t *c,
    enum bridge_storage kept)
{
    storage_t *storage = m->b->storage;
    uint32_t carriers = storage_count_carriers(storage, c->type);
    uint32_t off = storage_count_off(storage, c->type);

    if (c->storage == BRIDGE_SINGLETON)
        return (abi_refuse(m, "type '%s' is a singleton, and is not moved to "
            "%s: there is no entity to carry it", c->name,
            ABI_STORAGE_NAMES[kept]));
    if (kept == BRIDGE_SINGLETON && carriers > 1)
        return (abi_refuse(m, "type '%s' is carried by %u entities, and is "
            "not moved to a singleton: nothing says which of them has its "
            "value", c->name, carriers));
    if (!off || abi_accepts(m, c->name, off))
        return (0);
    return (abi_refuse(m, "type '%s' is a toggle that is off for %u rows, and "
        "as %s they are carried like the rest. That is a loss this reload "
        "does not accept: '%s'", c->name, off, ABI_STORAGE_NAMES[kept],
        c->name));
}
