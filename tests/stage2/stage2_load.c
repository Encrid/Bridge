/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage2/stage2_load.c -- how this program states and loads a
** module, and the cases that load one as it is.
*/

#include <stdio.h>

#include "stage2.h"

/*
** stage2_print
**
** The sink of this program: prints a record's level and its text.
*/
void stage2_print([[maybe_unused]] void *ud, const bridge_record_t *r)
{
    printf("record[%d]: %.*s\n", r->level, (int)r->len, r->text);
}

/*
** stage2_state
**
** States the wasm module of a name as `test:<name>@0.1.0`, with the small
** limits this stage loads everything under. `identity` is 64 bytes the name
** is written in.
*/
void stage2_state(bridge_artifact_t *a, char *identity, const char *name)
{
    snprintf(identity, 64, "test:%s@0.1.0", name);
    host_state_wasm(a, identity, name);
    a->cfg.work_limit = 1'000'000;
    a->cfg.memory_limit = 1 << 20;
    a->cfg.init_timeout = 200 * HOST_MS;
    a->cfg.reload_timeout = 200 * HOST_MS;
    a->cfg.shutdown_timeout = 200 * HOST_MS;
    a->cfg.scratch = 4096;
    a->cfg.log_ring = 240;
    a->cfg.spans = 4096;
    a->cfg.changes = 4096;
}

/*
** stage2_load
**
** Loads the wasm module of a name, stated as stage2_state has it.
*/
void stage2_load(host_t *h, const char *name)
{
    char identity[64];
    bridge_artifact_t a;

    stage2_state(&a, identity, name);
    host_load(h, &a);
}

/*
** stage2_load_named
**
** Loads the module the case is named after.
*/
void stage2_load_named(host_t *h)
{
    if (host_open(h))
        return;
    stage2_load(h, h->what);
    host_close(h);
}

/*
** stage2_stop
**
** A module that fails stops the bridge if it stated fail, and does not if
** it stated skip: what is loaded after it says which.
*/
void stage2_stop(host_t *h)
{
    char identity[64];
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage2_load(h, "oob");
    stage2_load(h, "ok");
    stage2_state(&a, identity, "oob");
    a.cfg.on_error = BRIDGE_FAIL;
    host_load(h, &a);
    stage2_load(h, "ok");
    host_close(h);
}
