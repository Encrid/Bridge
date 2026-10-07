/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage6/stage6_fail.c -- a load that fails, and a system that fails
** every pass until its module is stopped.
**
** failure.md, *When something fails*
*/

#include "stage6.h"

/*
** stage6_skip_init
**
** A failed init, skipped: the rest runs on.
*/
void stage6_skip_init(host_t *h)
{
    if (host_open(h))
        return;
    stage6_load_wasm(h, "game:twostep@0.1.0", "twostep");
    stage6_load_wasm(h, "game:badobs@0.1.0", "badobs");
    host_run_frames(h, 1);
    stage6_say_modules(h);
    host_close(h);
}

/*
** stage6_refuse_load
**
** Nothing of a refused load is left.
*/
void stage6_refuse_load(host_t *h)
{
    if (host_open(h))
        return;
    stage6_load_wasm(h, "game:badterm@0.1.0", "badterm");
    stage6_load_wasm(h, "game:fp@0.1.0", "fp");
    stage6_say_modules(h);
    host_close(h);
}

/*
** fail_always
**
** Loads a module that runs and one whose system fails every pass, which
** states `on_error`, and runs five frames.
*/
static void fail_always(host_t *h, enum bridge_on_error on_error)
{
    bridge_artifact_t a;

    stage6_load_wasm(h, "game:twostep@0.1.0", "twostep");
    stage6_state_wasm(&a, "test:flaky@0.1.0", "flaky-always");
    a.cfg.spawns = BRIDGE_NONE;
    a.cfg.on_error = on_error;
    host_load(h, &a);
    host_run_frames(h, 5);
    stage6_say_modules(h);
}

/*
** stage6_reach_budget
**
** A module that states skip is stopped at its budget of failures, and the
** bridge runs on.
*/
void stage6_reach_budget(host_t *h)
{
    if (host_open(h))
        return;
    fail_always(h, BRIDGE_SKIP);
    host_close(h);
}

/*
** stage6_stop_by_failing
**
** A module that states fail stops the bridge at its budget. A stopped
** bridge refuses an unload as it refuses a phase and a load.
*/
void stage6_stop_by_failing(host_t *h)
{
    if (host_open(h))
        return;
    fail_always(h, BRIDGE_FAIL);
    host_unload(h, "game:twostep@0.1.0");
    host_close(h);
}
