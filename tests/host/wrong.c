/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/host/wrong.c -- the wrong module, in the build the case's row
** names: it does one thing an entry refuses, at init or in its system.
*/

#include "cases.h"

static const char *const TEST_THING[] = {
    "other:Thing"
};

/*
** test_do_wrong
**
** Loads a build of the wrong module, granted to read the thing of another
** namespace it declares, and runs a frame.
*/
void test_do_wrong(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    host_state_wasm(&a, "game:wrong@0.1.0", h->arg);
    a.cfg.read = TEST_THING;
    a.cfg.read_count = 1;
    host_load(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}
