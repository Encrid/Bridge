/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage6/stage6_repair.c -- a stopped bridge, and the one call it
** accepts: the one that brings back the module that stopped it.
**
** failure.md, *When something fails*
*/

#include "stage6.h"

/*
** stage6_repair
**
** An essential module stopped at its budget stops the bridge; another
** module is refused; reloading the first into one that works is the one
** thing a stopped bridge accepts, and it runs again.
*/
void stage6_repair(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage6_state_ticker(&a, "game:ticker@0.1.0", "ticker-trip");
    a.cfg.on_error = BRIDGE_FAIL;
    host_load(h, &a);
    host_run_frames(h, 4);
    stage6_load_wasm(h, "game:twostep@0.1.0", "twostep");
    stage6_state_ticker(&a, "game:ticker@0.2.0", "ticker-ten");
    a.cfg.on_error = BRIDGE_FAIL;
    host_reload(h, &a);
    host_run_frames(h, 2);
    stage6_say_modules(h);
    host_close(h);
}

/*
** stage6_repair_by_load
**
** The same when what stopped it was a load: loading that module again, from
** an artifact that works, lifts it.
*/
void stage6_repair_by_load(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage6_state_wasm(&a, "game:thing@0.1.0", "badobs");
    a.cfg.on_error = BRIDGE_FAIL;
    host_load(h, &a);
    stage6_load_wasm(h, "game:other@0.1.0", "twostep");
    stage6_state_wasm(&a, "game:thing@0.2.0", "twostep");
    a.cfg.on_error = BRIDGE_FAIL;
    host_load(h, &a);
    host_run_frames(h, 1);
    stage6_say_modules(h);
    host_close(h);
}

/*
** stage6_allow_not_reloadable
**
** The deployment allows what the author says cannot be done.
*/
void stage6_allow_not_reloadable(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage6_state_wasm(&a, "game:twostep@0.1.0", "twostep");
    a.cfg.hot_reload = BRIDGE_TRUE;
    host_load(h, &a);
    host_close(h);
}

/*
** stage6_leave_reload_out
**
** One of the three lifecycle exports is missing.
*/
void stage6_leave_reload_out(host_t *h)
{
    if (host_open(h))
        return;
    stage6_load_spawnless(h, "test:noreload@0.1.0", "noreload");
    host_close(h);
}
