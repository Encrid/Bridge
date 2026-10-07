/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/host/sdk_wasm.c -- what a wasm module's binding does that no other
** case shows: a line at each severity, a tag, a scratch arena asked for
** more than it has, and a pull of fewer modules than are loaded.
*/

#include <stdio.h>
#include <stdlib.h>

#include "cases.h"

/*
** test_log_levels
**
** The module that writes a line at each severity, with the log_level the
** case's row states.
*/
void test_log_levels(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    host_state_wasm(&a, "test:levels@0.1.0", "levels");
    a.cfg.log_level = (uint8_t)atoi(h->arg);
    host_load(h, &a);
    (void)bridge_run_phase(h->b, "update");
    host_close(h);
}

/*
** test_tag_entities
**
** The module that freezes two of its three entities with a tag.
*/
void test_tag_entities(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    host_state_wasm(&a, "game:tagged@0.1.0", "tagged");
    a.cfg.spawns = 8;
    host_load(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}

/*
** test_take_scratch_past
**
** The mover, with a scratch arena of 64 bytes: what its init states does
** not fit, and the binding stops it there.
*/
void test_take_scratch_past(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_module(&a, "game:mover@0.1.0", "mover");
    a.cfg.scratch = 64;
    host_load(h, &a);
    host_close(h);
}

/*
** test_list_fewer
**
** Two modules loaded, and a pull with room for one: it answers two, and
** fills one.
*/
void test_list_fewer(host_t *h)
{
    bridge_module_info_t info[2] = { { "untouched", BRIDGE_RUNNING },
        { "untouched", BRIDGE_RUNNING } };
    int n;

    if (host_open(h))
        return;
    test_load_small(h, "ok");
    test_load_small(h, "image");
    n = bridge_list_modules(h->b, info, 1);
    printf("%d loaded, and of two rows: %s, %s\n", n, info[0].identity,
        info[1].identity);
    host_close(h);
}
