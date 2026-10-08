/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/state_ticker.c -- how this program states a module.
*/

#include "cases.h"

/*
** test_get_noted
**
** Answers what this program keeps beside the host.
*/
test_noted_t *test_get_noted(const host_t *h)
{
    return (h->ud);
}

/*
** test_state_native_minting
**
** States a native module that mints up to 32 ids between two barriers.
*/
void test_state_native_minting(bridge_artifact_t *a, const char *identity,
    const char *path)
{
    host_state_native(a, identity, path);
    a->cfg.spawns = 32;
}

/*
** test_state_wasm_minting
**
** States a wasm module that mints up to 32 ids between two barriers.
*/
void test_state_wasm_minting(bridge_artifact_t *a, const char *identity,
    const char *file)
{
    host_state_wasm(a, identity, file);
    a->cfg.spawns = 32;
}

/*
** test_state_ticker
**
** States a wasm build of the ticker that is reloadable, and may be
** reloaded.
*/
void test_state_ticker(bridge_artifact_t *a, const char *identity,
    const char *file)
{
    test_state_wasm_minting(a, identity, file);
    a->cfg.hot_reload = BRIDGE_TRUE;
    a->cfg.reloadable = BRIDGE_TRUE;
}

/*
** test_state_configured
**
** States the settings module, with the program's settings and the
** component they seed.
*/
void test_state_configured(const host_t *h, bridge_artifact_t *a,
    const char *identity)
{
    test_state_wasm_minting(a, identity, "settings");
    a->cfg.config = "game:Settings";
    a->cfg.settings = test_get_noted(h)->settings;
    a->cfg.setting_count = 10;
    a->cfg.persistent = 64;
}
