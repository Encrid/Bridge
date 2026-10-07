/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage6/stage6_shutdown.c -- a module's last call, and the record a
** module takes of the budget while it is loaded.
**
** lifecycle.md, *The runtime lifecycle*
*/

#include "stage6.h"

static void load_hang(host_t *h)
{
    bridge_artifact_t a;

    stage6_state_wasm(&a, "test:hang@0.1.0", "hang");
    a.cfg.spawns = BRIDGE_NONE;
    a.cfg.shutdown_timeout = 100 * HOST_MS;
    host_load(h, &a);
}

/*
** stage6_destroy_loaded
**
** A bridge destroyed with modules loaded shuts each of them down first.
*/
void stage6_destroy_loaded(host_t *h)
{
    if (host_open(h))
        return;
    stage6_load_wasm(h, "game:ticker@0.1.0", "ticker");
    load_hang(h);
    host_run_frames(h, 1);
    host_close(h);
}

/*
** stage6_time_out_shutdown
**
** A shutdown that never returns.
*/
void stage6_time_out_shutdown(host_t *h)
{
    if (host_open(h))
        return;
    load_hang(h);
    host_unload(h, "test:hang@0.1.0");
    stage6_say_modules(h);
    host_close(h);
}

/*
** stage6_take_slot
**
** Room for one module: a record is taken at load and given back at unload.
*/
void stage6_take_slot(host_t *h)
{
    h->cfg.budget.modules = 1;
    if (host_open(h))
        return;
    stage6_load_wasm(h, "game:twostep@0.1.0", "twostep");
    stage6_load_wasm(h, "game:ticker@0.1.0", "ticker");
    host_unload(h, "game:twostep@0.1.0");
    stage6_load_wasm(h, "game:ticker@0.1.0", "ticker");
    stage6_say_modules(h);
    host_close(h);
}

/*
** stage6_state_native_timeout
**
** A timeout is a deadline, which the native backend lacks.
*/
void stage6_state_native_timeout(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage6_state_native(&a, "game:ticker@0.1.0", "no-such-library");
    a.cfg.shutdown_timeout = 100 * HOST_MS;
    host_load(h, &a);
    host_close(h);
}
