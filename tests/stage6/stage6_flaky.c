/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage6/stage6_flaky.c -- the count of a system's failures: it
** falls as fast as it rises.
**
** failure.md, *The budget*
*/

#include "stage6.h"

/*
** stage6_give_up
**
** A stopped bridge takes an unload of the module that stopped it: the host
** goes on without it.
*/
void stage6_give_up(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage6_load_wasm(h, "game:twostep@0.1.0", "twostep");
    stage6_state_wasm(&a, "test:flaky@0.1.0", "flaky-always");
    a.cfg.spawns = BRIDGE_NONE;
    a.cfg.on_error = BRIDGE_FAIL;
    host_load(h, &a);
    host_run_frames(h, 5);
    stage6_say_modules(h);
    host_unload(h, "game:twostep@0.1.0");
    host_unload(h, "test:flaky@0.1.0");
    host_run_frames(h, 1);
    stage6_say_modules(h);
    host_close(h);
}

/*
** run_flaky
**
** Loads one build of the flaky module, which stops after `limit` failures,
** runs it for some frames and says what is loaded.
*/
static void run_flaky(host_t *h, const char *file, int frames, int64_t limit)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage6_state_wasm(&a, "test:flaky@0.1.0", file);
    a.cfg.spawns = BRIDGE_NONE;
    a.cfg.max_system_failures = limit;
    host_load(h, &a);
    host_run_frames(h, frames);
    stage6_say_modules(h);
    host_close(h);
}

/*
** stage6_forgive
**
** One pass in three fails: the count never reaches the budget.
*/
void stage6_forgive(host_t *h)
{
    run_flaky(h, "flaky-third", 12, 3);
}

/*
** stage6_forgive_too_little
**
** Two passes in three fail: the count reaches the budget.
*/
void stage6_forgive_too_little(host_t *h)
{
    run_flaky(h, "flaky-twothirds", 12, 3);
}

/*
** stage6_fail_beside_clean
**
** A system that always fails, and one in the next phase that never does:
** the clean one does not forgive the other.
*/
void stage6_fail_beside_clean(host_t *h)
{
    run_flaky(h, "flaky-calm", 5, 3);
}
