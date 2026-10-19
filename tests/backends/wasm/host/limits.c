/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

void test_lend_scratch(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_module(&a, "game:mover@0.1.0", "mover");
    a.cfg.scratch = 512;
    host_load(h, &a);
    host_run_frames(h, 2);
    host_close(h);
}

void test_spend_work(host_t *h)
{
    if (host_open(h))
        return;
    test_load_module(h, "test:spin@0.1.0", "spin3");
    host_run_frames(h, 2);
    host_close(h);
}

void test_spawn_to_ceiling(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_module(&a, "test:flood@0.1.0", "flood");
    a.cfg.spawns = 3000;
    host_load(h, &a);
    host_run_frames(h, 2);
    host_close(h);
}

void test_fill_ring(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_module(&a, "test:chatty@0.1.0", "chatty");
    a.cfg.log_ring = 16 * 24;
    host_load(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}

void test_overflow_spans(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_load_mover(h);
    test_state_reader(&a);
    a.cfg.spans = 16;
    host_load(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}
