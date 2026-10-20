/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "module.h"

static int check_kept(const module_check_t *k, uint32_t flag, const char *key)
{
    if (k->on->capabilities & flag)
        return (0);
    return (module_refuse(k->b, k->id, "it states %s, which backend '%s' "
        "cannot keep", key, k->on->name));
}

static int check_limits_kept(const module_check_t *k)
{
    const bridge_module_config_t *cfg = k->cfg;

    return ((cfg->work_limit > 0
            && check_kept(k, BACKEND_WORK_LIMIT, "work_limit"))
        || (cfg->memory_limit > 0
            && check_kept(k, BACKEND_MEMORY_LIMIT, "memory_limit"))
        || (cfg->init_timeout > 0
            && check_kept(k, BACKEND_TIMEOUT, "init_timeout"))
        || (cfg->reload_timeout > 0
            && check_kept(k, BACKEND_TIMEOUT, "reload_timeout"))
        || (cfg->shutdown_timeout > 0
            && check_kept(k, BACKEND_TIMEOUT, "shutdown_timeout"))
        || (cfg->hot_reload == BRIDGE_TRUE
            && check_kept(k, BACKEND_RELOAD, "hot_reload")));
}

int module_check_backend(const module_check_t *k)
{
    const bridge_module_config_t *cfg = k->cfg;

    if (check_limits_kept(k))
        return (1);
    if (cfg->hot_reload == BRIDGE_TRUE && cfg->reloadable == BRIDGE_FALSE)
        return (module_refuse(k->b, k->id, "hot_reload is stated, and its "
            "author states it is not reloadable"));
    return (0);
}
