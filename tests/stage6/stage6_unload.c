/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage6/stage6_unload.c -- what an unload leaves behind, by the
** policy the component's last declarer stated.
**
** lifetime.md, *What unload leaves behind*
*/

#include <stdio.h>

#include "stage6.h"

static const char *const STAGE6_ROSTER[] = {
    "game:Position",
    "game:Roster"
};

/*
** stage6_fail_unlimited
**
** A budget of failures stated as no limit: the system fails every pass and
** its module is never stopped.
*/
void stage6_fail_unlimited(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage6_state_wasm(&a, "test:flaky@0.1.0", "flaky-always");
    a.cfg.spawns = BRIDGE_NONE;
    a.cfg.max_system_failures = BRIDGE_NO_LIMIT;
    host_load(h, &a);
    host_run_frames(h, 8);
    stage6_say_modules(h);
    host_close(h);
}

/*
** load_keeper
**
** Loads the keeper built with the policy the case is named after, and the
** counter, which reads what the keeper leaves.
*/
static void load_keeper(host_t *h)
{
    char file[32];
    bridge_artifact_t a;

    snprintf(file, sizeof(file), "keeper-%s", h->what);
    stage6_load_wasm(h, "game:keeper@0.1.0", file);
    stage6_state_wasm(&a, "view:counter@0.1.0", "counter");
    a.cfg.read = STAGE6_ROSTER;
    a.cfg.read_count = 2;
    a.cfg.spawns = BRIDGE_NONE;
    host_load(h, &a);
}

/*
** stage6_unload_keeper
**
** The keeper is unloaded, and its private component goes by its policy.
** The component only the keeper declared is gone: its name is free for
** another layout.
*/
void stage6_unload_keeper(host_t *h)
{
    if (host_open(h))
        return;
    load_keeper(h);
    host_run_frames(h, 1);
    host_unload(h, "game:keeper@0.1.0");
    host_run_frames(h, 1);
    stage6_load_spawnless(h, "game:retype@0.1.0", "retype-private");
    stage6_say_modules(h);
    host_close(h);
}

/*
** stage6_unload_declarer
**
** The declarer of a type goes and its user stays: the type is still held,
** and another layout of it is refused until the user goes too.
*/
void stage6_unload_declarer(host_t *h)
{
    if (host_open(h))
        return;
    stage6_load_spawnless(h, "game:typer@0.1.0", "typer");
    stage6_load_wasm(h, "game:user@0.1.0", "user");
    host_unload(h, "game:typer@0.1.0");
    host_run_frames(h, 1);
    stage6_load_spawnless(h, "game:retype@0.1.0", "retype-vec2");
    host_unload(h, "game:user@0.1.0");
    stage6_load_spawnless(h, "game:retype@0.1.0", "retype-vec2");
    host_close(h);
}

/*
** stage6_unload_nobody
**
** An unload of a module that is not loaded.
*/
void stage6_unload_nobody(host_t *h)
{
    if (host_open(h))
        return;
    host_unload(h, "game:nobody@0.1.0");
    host_close(h);
}
