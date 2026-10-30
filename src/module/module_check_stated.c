/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "module.h"

int module_check_flags(const module_check_t *k)
{
    const bridge_module_config_t *cfg = k->cfg;

    return (module_check_flag(k, "reloadable", cfg->reloadable)
        || module_check_flag(k, "hot_reload", cfg->hot_reload)
        || module_check_flag(k, "pinned", cfg->pinned));
}

static int check_limits_set(const module_check_t *k)
{
    const bridge_module_config_t *cfg = k->cfg;

    return (module_check_set(k, "max_system_failures",
            cfg->max_system_failures)
        || module_check_set(k, "work_limit", cfg->work_limit)
        || module_check_set(k, "memory_limit", cfg->memory_limit)
        || module_check_set(k, "element_limit", cfg->element_limit)
        || module_check_set(k, "init_timeout", cfg->init_timeout)
        || module_check_set(k, "reload_timeout", cfg->reload_timeout)
        || module_check_set(k, "shutdown_timeout", cfg->shutdown_timeout));
}

static int check_amounts_set(const module_check_t *k)
{
    const bridge_module_config_t *cfg = k->cfg;

    return (module_check_set(k, "scratch", cfg->scratch)
        || module_check_set(k, "log_ring", cfg->log_ring)
        || module_check_set(k, "spans", cfg->spans)
        || module_check_set(k, "spawns", cfg->spawns)
        || module_check_set(k, "persistent", cfg->persistent)
        || module_check_set(k, "changes", cfg->changes)
        || module_check_set(k, "loss_count", cfg->loss_count));
}

static int check_bounds(const module_check_t *k)
{
    const bridge_module_config_t *cfg = k->cfg;
    constexpr int64_t limit = BRIDGE_NO_LIMIT;
    constexpr int64_t none = BRIDGE_NONE;

    return (module_check_bound(k, "max_system_failures",
            cfg->max_system_failures, limit)
        || module_check_bound(k, "work_limit", cfg->work_limit, limit)
        || module_check_bound(k, "memory_limit", cfg->memory_limit, limit)
        || module_check_bound(k, "element_limit", cfg->element_limit, limit)
        || module_check_bound(k, "init_timeout", cfg->init_timeout, limit)
        || module_check_bound(k, "reload_timeout", cfg->reload_timeout, limit)
        || module_check_bound(k, "shutdown_timeout", cfg->shutdown_timeout,
            limit)
        || module_check_bound(k, "spawns", cfg->spawns, none)
        || module_check_bound(k, "persistent", cfg->persistent, none)
        || module_check_bound(k, "changes", cfg->changes, none)
        || module_check_bound(k, "loss_count", cfg->loss_count, none));
}

int module_check_stated(const module_check_t *k)
{
    const bridge_module_config_t *cfg = k->cfg;

    if (cfg->on_error != BRIDGE_FAIL && cfg->on_error != BRIDGE_SKIP)
        return (module_refuse(k->w, k->id, "on_error is unset"));
    if (check_limits_set(k) || check_amounts_set(k))
        return (1);
    if (cfg->loss_count > 0 && !cfg->losses)
        return (module_refuse(k->w, k->id, "it states %lld losses, and "
            "losses is unset", (long long)cfg->loss_count));
    if (check_bounds(k))
        return (1);
    if (cfg->changes > (int64_t)UINT32_MAX)
        return (module_refuse(k->w, k->id, "its changes of %lld bytes is "
            "more than an offset of 32 bits can reach",
            (long long)cfg->changes));
    return (0);
}
