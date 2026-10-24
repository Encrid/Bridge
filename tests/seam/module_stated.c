/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "cases.h"

void test_state_bad_level(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_module(&a, "game:mover@0.1.0", "mover");
    a.cfg.log_level = 8;
    host_load(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}

void test_log_warnings(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_module(&a, "game:mover@0.1.0", "mover");
    a.cfg.log_level = BRIDGE_WARN;
    host_load(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}

void test_state_big_private(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_module(&a, "game:mover@0.1.0", "mover");
    a.cfg.scratch = 0xffffff00u;
    a.cfg.spans = 4096;
    host_load(h, &a);
    host_close(h);
}

void test_state_no_spawns(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_module(&a, "game:mover@0.1.0", "mover");
    a.cfg.spawns = BRIDGE_NONE;
    host_load(h, &a);
    host_close(h);
}

void test_run_unknown_phase(host_t *h)
{
    if (host_open(h))
        return;
    if (bridge_run_phase(h->w, "render"))
        printf("render refused\n");
    host_close(h);
}
