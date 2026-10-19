/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

test_noted_t *test_get_noted(const host_t *h)
{
    return (h->ud);
}

void test_state_wasm_minting(bridge_artifact_t *a, const char *identity,
    const char *file)
{
    host_state_wasm(a, identity, file);
    a->cfg.spawns = 32;
}

void test_state_ticker(bridge_artifact_t *a, const char *identity,
    const char *file)
{
    test_state_wasm_minting(a, identity, file);
    a->cfg.hot_reload = BRIDGE_TRUE;
    a->cfg.reloadable = BRIDGE_TRUE;
}

void test_state_configured(const host_t *h, bridge_artifact_t *a,
    const char *identity)
{
    test_state_wasm_minting(a, identity, "settings");
    a->cfg.config = "game:Settings";
    a->cfg.settings = test_get_noted(h)->settings;
    a->cfg.setting_count = 10;
    a->cfg.persistent = 64;
}
