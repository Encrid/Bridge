/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage4/stage4_refused.c -- what a backend cannot keep or cannot
** find, refused at the load.
**
** capabilities.md, *Nothing is downgraded*
*/

#include "stage4.h"

/*
** stage4_state_untrusted
**
** A module that is not trusted, on a backend that runs only modules that
** are: refused before any library is looked for.
*/
void stage4_state_untrusted(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage4_state(&a, "game:mover@0.1.0", "no-such-library");
    a.cfg.trusted = BRIDGE_FALSE;
    host_load(h, &a);
    host_close(h);
}

/*
** stage4_state_limit
**
** A work limit, on a backend that keeps none.
*/
void stage4_state_limit(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage4_state(&a, "game:mover@0.1.0", "no-such-library");
    a.cfg.work_limit = 1000;
    host_load(h, &a);
    host_close(h);
}

/*
** stage4_state_no_path
**
** A native module, and nowhere to find it.
*/
void stage4_state_no_path(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage4_state(&a, "game:mover@0.1.0", nullptr);
    host_load(h, &a);
    host_close(h);
}

/*
** stage4_state_no_bytes
**
** A wasm module, and nothing to compile.
*/
void stage4_state_no_bytes(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage4_state_reader(&a);
    a.bytes = nullptr;
    a.len = 0;
    host_load(h, &a);
    host_close(h);
}

/*
** stage4_name_no_library
**
** A native module whose library cannot be opened.
*/
void stage4_name_no_library(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage4_state(&a, "game:mover@0.1.0", "no-such-library");
    host_load(h, &a);
    host_close(h);
}
