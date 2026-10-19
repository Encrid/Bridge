/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "host.h"

static enum bridge_flag get_flag(bool yes)
{
    return (yes ? BRIDGE_TRUE : BRIDGE_FALSE);
}

bool state_found(const stating_t *s, bridge_module_config_t *cfg)
{
    const manifest_t *who = &s->of->who;
    bool taken = state_own_keys(s);

    *cfg = (bridge_module_config_t){ .identity = who->identity,
        .abi = BRIDGE_ABI, .config = who->config,
        .pinned = get_flag(who->pinned),
        .reloadable = get_flag(who->reloadable) };
    for (uint32_t i = 0; i < MODULE_KEY_COUNT; i++)
        taken = state_take_key(s, &MODULE_KEYS[i], cfg) && taken;
    taken = state_take_phases(s, cfg) && taken;
    return (state_settings(s, cfg) && taken);
}
