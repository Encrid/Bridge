/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>

#include "cases.h"

void test_lend_scratch(host_t *h)
{
    char file[32] = "";
    uint32_t scratch = 0;
    bridge_artifact_t a;

    if (host_open(h))
        return;
    (void)sscanf(h->arg, "%31s %u", file, &scratch);
    test_state_module(&a, "game:mover@0.1.0", file);
    a.cfg.scratch = scratch;
    host_load(h, &a);
    host_run_frames(h, 2);
    host_close(h);
}

void test_spend_work(host_t *h)
{
    if (host_open(h))
        return;
    test_load_module(h, "test:spin@0.1.0", h->arg);
    host_run_frames(h, 2);
    host_close(h);
}

void test_spawn_to_ceiling(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_module(&a, "test:flood@0.1.0", "flood");
    a.cfg.spawns = strtol(h->arg, nullptr, 10);
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
    a.cfg.log_ring = (uint32_t)strtol(h->arg, nullptr, 10);
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
    a.cfg.spans = (uint32_t)strtol(h->arg, nullptr, 10);
    host_load(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}
