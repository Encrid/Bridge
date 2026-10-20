/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stddef.h>

#include "bridge.h"
#include "config.h"

const module_rule_t MODULE_KEYS[] = {
    { "hot_reload", CHECK_FLAG, 1,
        offsetof(bridge_module_config_t, hot_reload), 0 },
    { "on_error", CHECK_ERROR, 1,
        offsetof(bridge_module_config_t, on_error), 0 },
    { "max_system_failures", CHECK_LIMIT, 8,
        offsetof(bridge_module_config_t, max_system_failures),
        BRIDGE_NO_LIMIT },
    { "work_limit", CHECK_LIMIT, 8,
        offsetof(bridge_module_config_t, work_limit), BRIDGE_NO_LIMIT },
    { "memory_limit", CHECK_LIMIT, 8,
        offsetof(bridge_module_config_t, memory_limit), BRIDGE_NO_LIMIT },
    { "init_timeout", CHECK_LIMIT, 8,
        offsetof(bridge_module_config_t, init_timeout), BRIDGE_NO_LIMIT },
    { "reload_timeout", CHECK_LIMIT, 8,
        offsetof(bridge_module_config_t, reload_timeout), BRIDGE_NO_LIMIT },
    { "shutdown_timeout", CHECK_LIMIT, 8,
        offsetof(bridge_module_config_t, shutdown_timeout), BRIDGE_NO_LIMIT },
    { "log_level", CHECK_LEVEL, 1,
        offsetof(bridge_module_config_t, log_level), 0 },
    { "scratch", CHECK_COUNT, 4,
        offsetof(bridge_module_config_t, scratch), 0 },
    { "log_ring", CHECK_COUNT, 4,
        offsetof(bridge_module_config_t, log_ring), 0 },
    { "spans", CHECK_COUNT, 4, offsetof(bridge_module_config_t, spans), 0 },
    { "spawns", CHECK_LIMIT, 8, offsetof(bridge_module_config_t, spawns),
        BRIDGE_NONE },
    { "persistent", CHECK_LIMIT, 8,
        offsetof(bridge_module_config_t, persistent), BRIDGE_NONE },
    { "changes", CHECK_LIMIT, 8, offsetof(bridge_module_config_t, changes),
        BRIDGE_NONE },
    { "losses", CHECK_NAMES_NONE, 0, 0, BRIDGE_NONE },
    { "read", CHECK_NAMES, 0, 0, 0 },
    { "write", CHECK_NAMES, 0, 0, 0 },
    { "abi", CHECK_AUTHOR, 0, 0, 0 },
    { "config", CHECK_AUTHOR, 0, 0, 0 },
    { "settings", CHECK_AUTHOR, 0, 0, 0 },
    { "pinned", CHECK_AUTHOR, 0, 0, 0 },
    { "reloadable", CHECK_AUTHOR, 0, 0, 0 }
};

const uint32_t MODULE_KEY_COUNT = sizeof(MODULE_KEYS) / sizeof(MODULE_KEYS[0]);
