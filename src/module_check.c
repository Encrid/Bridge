/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** src/module_check.c -- what can be decided from a config alone: everything
** a load and a reload both refuse before the artifact is touched.
*/

#include <string.h>

#include "module.h"

/*
** module_find
**
** Answers the loaded module of an identity, whatever its version.
*/
module_t *module_find(bridge_t *b, const char *identity)
{
    size_t stem = bridge_get_stem_length(identity);
    module_t *m = b->modules;

    while (m && (strlen(m->stem) != stem || memcmp(m->stem, identity, stem)))
        m = m->next;
    return (m);
}

static open_backend_t *find_backend(bridge_t *b, const char *name)
{
    open_backend_t *o = nullptr;
    uint32_t i = 0;

    while (name && i < b->backend_count) {
        if (!strcmp(b->backends[i].name, name))
            o = &b->backends[i];
        i++;
    }
    return (o);
}

/*
** check_whose
**
** Refuses an artifact with no identity, one that names no backend there is,
** and one built for another ABI.
**
** module.md, *Identity*
** lifecycle.md, *The runtime lifecycle*
*/
static int check_whose(module_check_t *k)
{
    const char *backend = k->a->backend;

    if (!k->id || !*k->id) {
        say_error(k->b, "a module with no identity is refused");
        return (1);
    }
    k->on = find_backend(k->b, backend);
    if (!k->on)
        return (module_refuse(k->b, k->id, "no backend named '%s'",
            backend ? backend : ""));
    if (k->cfg->abi != BRIDGE_ABI)
        return (module_refuse(k->b, k->id, "it wants ABI %u and this is ABI "
            "%u", k->cfg->abi, (unsigned)BRIDGE_ABI));
    return (0);
}

/*
** module_check_room
**
** Refuses a module that is loaded already, that is one more than the budget
** of modules, or whose share does not fit in what is left of the persistent
** arena.
**
** schedule.md, *Order within a phase is never implicit*
** budgets.md, *Budgets*
*/
int module_check_room(const module_check_t *k)
{
    bridge_t *b = k->b;
    const module_t *other = module_find(b, k->id);
    int64_t share = k->cfg->persistent;
    uint32_t left = region_get_most(&b->arena_space);

    if (other)
        return (module_refuse(b, k->id, "'%s' is loaded already",
            other->identity));
    if (b->module_count >= b->max_modules)
        return (module_refuse(b, k->id, "[budget] modules of %u is reached",
            b->max_modules));
    if (share > 0 && (uint64_t)share > left)
        return (module_refuse(b, k->id, "its persistent of %lld bytes does "
            "not fit in the %u left of [budget] persistent", (long long)share,
            left));
    return (0);
}

/*
** module_check
**
** Refuses everything a load and a reload both refuse before the artifact is
** touched, and works out what its config demands of its backend.
*/
int module_check(module_check_t *k)
{
    const bridge_module_config_t *cfg = &k->a->cfg;

    k->cfg = cfg;
    k->id = cfg->identity;
    k->demands = 0;
    if (check_whose(k) || module_check_flags(k) || module_check_phases(k))
        return (1);
    if (module_check_stated(k) || module_check_fit(k)
        || module_check_backend(k))
        return (1);
    if (cfg->trusted != BRIDGE_TRUE)
        k->demands |= BACKEND_UNTRUSTED;
    if (cfg->work_limit > 0)
        k->demands |= BACKEND_WORK_LIMIT;
    if (cfg->memory_limit > 0)
        k->demands |= BACKEND_MEMORY_LIMIT;
    if (cfg->init_timeout > 0 || cfg->reload_timeout > 0
        || cfg->shutdown_timeout > 0)
        k->demands |= BACKEND_TIMEOUT;
    return (0);
}
