/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "module.h"

module_t *module_find(bridge_world_t *w, const char *identity)
{
    size_t stem = bridge_get_stem_length(identity);
    module_t *m = w->modules;

    while (m && (strlen(m->stem) != stem || memcmp(m->stem, identity, stem)))
        m = m->next;
    return (m);
}

static open_backend_t *find_backend(bridge_world_t *w, const char *name)
{
    open_backend_t *o = nullptr;
    uint32_t i = 0;

    while (name && i < w->backend_count) {
        if (!strcmp(w->backends[i].name, name))
            o = &w->backends[i];
        i++;
    }
    return (o);
}

static int check_whose(module_check_t *k)
{
    const char *backend = k->a->backend;

    if (module_check_identity(k->w, k->id))
        return (1);
    k->on = find_backend(k->w, backend);
    if (!k->on)
        return (module_refuse(k->w, k->id, "no backend named '%s'",
            backend ? backend : ""));
    if (k->cfg->abi != BRIDGE_ABI)
        return (module_refuse(k->w, k->id, "it wants ABI %u and this is ABI "
            "%u", k->cfg->abi, (unsigned)BRIDGE_ABI));
    return (0);
}

int module_check_room(const module_check_t *k)
{
    bridge_world_t *w = k->w;
    const module_t *other = module_find(w, k->id);
    uint64_t share = module_round_share(k->cfg->persistent);
    uint32_t left = region_get_most(&w->arena_space);

    if (other)
        return (module_refuse(w, k->id, "'%s' is loaded already",
            other->identity));
    if (w->module_count >= w->max_modules)
        return (module_refuse(w, k->id, "[budget] modules of %u is reached",
            w->max_modules));
    if (share > left)
        return (module_refuse(w, k->id, "its persistent of %lld bytes does "
            "not fit in the %u left of [budget] persistent",
            (long long)k->cfg->persistent, left));
    return (0);
}

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
    if (cfg->work_limit > 0)
        k->demands |= BACKEND_WORK_LIMIT;
    if (cfg->memory_limit > 0)
        k->demands |= BACKEND_MEMORY_LIMIT;
    if (cfg->init_timeout > 0 || cfg->reload_timeout > 0
        || cfg->shutdown_timeout > 0)
        k->demands |= BACKEND_TIMEOUT;
    return (0);
}
