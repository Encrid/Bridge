/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_inside.h"
#include "say.h"

static int refuse_unset(bridge_world_t *w, const char *key, uint64_t value)
{
    if (value)
        return (0);
    say_error(w, "%s is unset", key);
    return (1);
}

static int refuse_flag(bridge_world_t *w, const char *key,
    enum bridge_flag value)
{
    if (refuse_unset(w, key, value))
        return (1);
    if (value == BRIDGE_TRUE || value == BRIDGE_FALSE)
        return (0);
    say_error(w, "%s is %u, which is neither BRIDGE_TRUE nor BRIDGE_FALSE",
        key, (unsigned)value);
    return (1);
}

static int refuse_growth(bridge_world_t *w, uint32_t value)
{
    if (refuse_unset(w, "table_growth", value))
        return (1);
    if (value >= 100)
        return (0);
    say_error(w, "table_growth is %u, which is less than 100", value);
    return (1);
}

static int refuse_share(bridge_world_t *w, const char *key, uint32_t value)
{
    if (refuse_unset(w, key, value))
        return (1);
    if (value < 100)
        return (0);
    say_error(w, "%s is %u, which is not 1 to 99", key, value);
    return (1);
}

int bridge_take_keys(bridge_world_t *w, const bridge_config_t *cfg)
{
    if (refuse_flag(w, "profile", cfg->profile)
        || refuse_flag(w, "validate", cfg->validate)
        || refuse_unset(w, "observer_passes", cfg->observer_passes)
        || refuse_growth(w, cfg->table_growth)
        || refuse_share(w, "table_hash_fill", cfg->table_hash_fill)
        || refuse_share(w, "table_retry", cfg->table_retry))
        return (1);
    w->profile = cfg->profile == BRIDGE_TRUE;
    w->validate = cfg->validate == BRIDGE_TRUE;
    w->observer_passes = cfg->observer_passes;
    w->table_growth = cfg->table_growth;
    w->table_hash_fill = cfg->table_hash_fill;
    w->table_retry = cfg->table_retry;
    return (0);
}
