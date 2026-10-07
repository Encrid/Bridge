/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage3/stage3_state.c -- how this program states and loads a
** module.
*/

#include <string.h>

#include "stage3.h"

static const char *const STAGE3_GRANTS[] = {
    "game:Position",
    "game:Marked"
};

/*
** stage3_state
**
** States the wasm module of a file under an identity, with what it mints
** and the changes it queues.
*/
void stage3_state(bridge_artifact_t *a, const char *identity,
    const char *file)
{
    host_state_wasm(a, identity, file);
    if (!strcmp(file, "mover"))
        a->cfg.spawns = 1024;
    if (!strcmp(file, "refs"))
        a->cfg.spawns = 128;
    if (!strcmp(file, "namer"))
        a->cfg.spawns = 8;
    if (!strncmp(file, "rows", 4))
        a->cfg.changes = 16 << 20;
}

/*
** stage3_load
**
** Loads the wasm module of a file under an identity.
*/
void stage3_load(host_t *h, const char *identity, const char *file)
{
    bridge_artifact_t a;

    stage3_state(&a, identity, file);
    host_load(h, &a);
}

/*
** stage3_load_mover
**
** Loads the mover, which owns game:Position and spawns the entities.
*/
void stage3_load_mover(host_t *h)
{
    stage3_load(h, "game:mover@0.1.0", "mover");
}

/*
** stage3_state_reader
**
** States the reader, of another namespace, with the grant to read the two
** components of the mover.
*/
void stage3_state_reader(bridge_artifact_t *a)
{
    stage3_state(a, "view:reader@0.1.0", "reader");
    a->cfg.read = STAGE3_GRANTS;
    a->cfg.read_count = 2;
}

/*
** stage3_load_reader
**
** Loads the reader with its grant.
*/
void stage3_load_reader(host_t *h)
{
    bridge_artifact_t a;

    stage3_state_reader(&a);
    host_load(h, &a);
}
