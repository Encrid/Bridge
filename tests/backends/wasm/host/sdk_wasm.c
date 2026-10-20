/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>

#include "cases.h"

void test_log_levels(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    host_state_wasm(&a, "test:levels@0.1.0", "levels");
    a.cfg.log_level = (uint8_t)atoi(h->arg);
    host_load(h, &a);
    (void)bridge_run_phase(h->w, "update");
    host_close(h);
}

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

void test_list_fewer(host_t *h)
{
    bridge_module_info_t info[2] = { { "untouched", BRIDGE_RUNNING },
        { "untouched", BRIDGE_RUNNING } };
    int n;

    if (host_open(h))
        return;
    test_load_small(h, "ok");
    test_load_small(h, "image");
    n = bridge_list_modules(h->w, info, 1);
    printf("%d loaded, and of two rows: %s, %s\n", n, info[0].identity,
        info[1].identity);
    host_close(h);
}
