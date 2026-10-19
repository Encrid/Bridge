/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "cases.h"

void test_configure(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_configured(h, &a, "game:settings@0.1.0");
    host_load(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}

void test_configure_twice(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_configured(h, &a, "game:settings@0.1.0");
    host_load(h, &a);
    test_state_configured(h, &a, "game:other@0.1.0");
    host_load(h, &a);
    host_close(h);
}

void test_reseed(host_t *h)
{
    const test_noted_t *more = test_get_noted(h);
    const uint32_t *at = more->path_at;
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_configured(h, &a, "game:settings@0.1.0");
    host_load(h, &a);
    host_run_frames(h, 1);
    host_unload(h, "game:settings@0.1.0");
    host_load(h, &a);
    host_run_frames(h, 1);
    printf("settings text %s\n", more->path_count == 2 && at[0] == at[1]
        ? "written to the same place" : "MOVED");
    host_close(h);
}

void test_reload_settings(host_t *h, bool drop)
{
    bridge_artifact_t a;

    test_state_configured(h, &a, "game:settings@0.1.0");
    a.cfg.hot_reload = BRIDGE_TRUE;
    a.cfg.reloadable = BRIDGE_TRUE;
    host_load(h, &a);
    host_run_frames(h, 1);
    test_get_noted(h)->settings[1].v.i = 640;
    test_state_configured(h, &a, "game:settings@0.2.0");
    if (drop)
        test_state_wasm_minting(&a, "game:settings@0.2.0", "settings-drop");
    a.cfg.persistent = 64;
    a.cfg.hot_reload = BRIDGE_TRUE;
    a.cfg.reloadable = BRIDGE_TRUE;
    host_reload(h, &a);
    host_run_frames(h, 1);
}

void test_reload_configured(host_t *h)
{
    if (host_open(h))
        return;
    test_reload_settings(h, false);
    host_close(h);
}
