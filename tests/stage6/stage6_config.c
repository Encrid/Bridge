/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage6/stage6_config.c -- a module's own settings: one row of the
** component its config names, written after its init.
**
** config.md, *A module's own settings are a component*
*/

#include <stdio.h>

#include "stage6.h"

/*
** stage6_configure
**
** The settings module, with a setting of every kind.
*/
void stage6_configure(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage6_state_configured(h, &a, "game:settings@0.1.0");
    host_load(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}

/*
** stage6_configure_twice
**
** One row, so one module: a second that names the component is refused.
*/
void stage6_configure_twice(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage6_state_configured(h, &a, "game:settings@0.1.0");
    host_load(h, &a);
    stage6_state_configured(h, &a, "game:other@0.1.0");
    host_load(h, &a);
    host_close(h);
}

/*
** stage6_reseed
**
** Unload, load: the text goes where it was.
*/
void stage6_reseed(host_t *h)
{
    const stage6_t *more = stage6_get(h);
    const uint32_t *at = more->path_at;
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage6_state_configured(h, &a, "game:settings@0.1.0");
    host_load(h, &a);
    host_run_frames(h, 1);
    host_unload(h, "game:settings@0.1.0");
    host_load(h, &a);
    host_run_frames(h, 1);
    printf("settings text %s\n", more->path_count == 2 && at[0] == at[1]
        ? "written to the same place" : "MOVED");
    host_close(h);
}

/*
** stage6_reload_settings
**
** Loads the settings module and reloads it with another width. A reload
** hands over other settings, and the row keeps what it had: after init the
** component is the state. With `drop` the new instance declares nothing,
** and the row goes with the component.
*/
void stage6_reload_settings(host_t *h, bool drop)
{
    bridge_artifact_t a;

    stage6_state_configured(h, &a, "game:settings@0.1.0");
    a.cfg.hot_reload = BRIDGE_TRUE;
    a.cfg.reloadable = BRIDGE_TRUE;
    host_load(h, &a);
    host_run_frames(h, 1);
    stage6_get(h)->settings[1].v.i = 640;
    stage6_state_configured(h, &a, "game:settings@0.2.0");
    if (drop)
        stage6_state_wasm(&a, "game:settings@0.2.0", "settings-drop");
    a.cfg.persistent = 64;
    a.cfg.hot_reload = BRIDGE_TRUE;
    a.cfg.reloadable = BRIDGE_TRUE;
    host_reload(h, &a);
    host_run_frames(h, 1);
}

/*
** stage6_reload_configured
**
** A reload with other settings: the row keeps what it had.
*/
void stage6_reload_configured(host_t *h)
{
    if (host_open(h))
        return;
    stage6_reload_settings(h, false);
    host_close(h);
}
