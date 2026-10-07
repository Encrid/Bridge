/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage6/stage6_state.c -- how this program states a module.
*/

#include "stage6.h"

/*
** stage6_get
**
** Answers what this program keeps beside the host.
*/
stage6_t *stage6_get(const host_t *h)
{
    return (h->ud);
}

/*
** stage6_state_native
**
** States a native module that mints up to 32 ids between two barriers.
*/
void stage6_state_native(bridge_artifact_t *a, const char *identity,
    const char *path)
{
    host_state_native(a, identity, path);
    a->cfg.spawns = 32;
}

/*
** stage6_state_wasm
**
** States a wasm module that mints up to 32 ids between two barriers.
*/
void stage6_state_wasm(bridge_artifact_t *a, const char *identity,
    const char *file)
{
    host_state_wasm(a, identity, file);
    a->cfg.spawns = 32;
}

/*
** stage6_state_ticker
**
** States a wasm build of the ticker that is reloadable, and may be
** reloaded.
*/
void stage6_state_ticker(bridge_artifact_t *a, const char *identity,
    const char *file)
{
    stage6_state_wasm(a, identity, file);
    a->cfg.hot_reload = BRIDGE_TRUE;
    a->cfg.reloadable = BRIDGE_TRUE;
}

/*
** stage6_state_configured
**
** States the settings module, with the program's settings and the
** component they seed.
*/
void stage6_state_configured(const host_t *h, bridge_artifact_t *a,
    const char *identity)
{
    stage6_state_wasm(a, identity, "settings");
    a->cfg.config = "game:Settings";
    a->cfg.settings = stage6_get(h)->settings;
    a->cfg.setting_count = 10;
    a->cfg.persistent = 64;
}
