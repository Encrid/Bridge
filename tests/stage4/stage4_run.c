/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage4/stage4_run.c -- how this program states a module, and the
** cases in which a native module runs.
*/

#include "stage4.h"

static const char *const STAGE4_GRANTS[] = {
    "game:Position",
    "game:Marked"
};

/*
** stage4_state
**
** States a native module that mints up to 1,024 ids between two barriers.
*/
void stage4_state(bridge_artifact_t *a, const char *identity,
    const char *path)
{
    host_state_native(a, identity, path);
    a->cfg.spawns = 1024;
}

/*
** stage4_state_reader
**
** States the reader of stage 3, on the wasm backend, with its grant.
*/
void stage4_state_reader(bridge_artifact_t *a)
{
    host_state_wasm(a, "view:reader@0.1.0", "reader");
    a->cfg.read = STAGE4_GRANTS;
    a->cfg.read_count = 2;
}

/*
** stage4_run_native
**
** The mover as a shared library, reading and writing through raw pointers.
*/
void stage4_run_native(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage4_state(&a, "game:mover@0.1.0", NMOVER_PATH);
    host_load(h, &a);
    host_run_frames_unsaid(h->b, 2);
    host_close(h);
}

/*
** stage4_run_mixed
**
** A wasm system in the phase of a native one.
*/
void stage4_run_mixed(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage4_state(&a, "game:mover@0.1.0", NMOVER_PATH);
    host_load(h, &a);
    stage4_state_reader(&a);
    host_load(h, &a);
    host_run_frames_unsaid(h->b, 2);
    host_close(h);
}

/*
** stage4_refuse_entry
**
** A native module an entry refuses: the invocation fails when it returns.
*/
void stage4_refuse_entry(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage4_state(&a, "test:flood@0.1.0", NFLOOD_PATH);
    host_load(h, &a);
    (void)bridge_run_phase(h->b, "update");
    host_close(h);
}
