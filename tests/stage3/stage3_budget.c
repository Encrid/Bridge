/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage3/stage3_budget.c -- what a module stated it needs, used up:
** its scratch, its work, its ids, its log ring, its spans.
**
** budgets.md, *What running out does*
*/

#include "stage3.h"

/*
** stage3_lend_scratch
**
** The mover's init attaches 1,024 positions, eight kilobytes of values,
** with a scratch of 512 bytes: a value's scratch is given back when the
** attach returns.
*/
void stage3_lend_scratch(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage3_state(&a, "game:mover@0.1.0", "mover");
    a.cfg.scratch = 512;
    host_load(h, &a);
    host_run_frames(h, 2);
    host_close(h);
}

/*
** stage3_spend_work
**
** A system that spins until its work limit ends it.
*/
void stage3_spend_work(host_t *h)
{
    if (host_open(h))
        return;
    stage3_load(h, "test:spin@0.1.0", "spin3");
    host_run_frames(h, 2);
    host_close(h);
}

/*
** stage3_spawn_to_ceiling
**
** 3,000 ids a barrier under a ceiling of 4,096: the first frame runs out of
** what was set aside for it, and the second of what the deployment has.
*/
void stage3_spawn_to_ceiling(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage3_state(&a, "test:flood@0.1.0", "flood");
    a.cfg.spawns = 3000;
    host_load(h, &a);
    host_run_frames(h, 2);
    host_close(h);
}

/*
** stage3_fill_ring
**
** A module that says more in a frame than its ring holds.
*/
void stage3_fill_ring(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage3_state(&a, "test:chatty@0.1.0", "chatty");
    a.cfg.log_ring = 16 * 24;
    host_load(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}

/*
** stage3_overflow_spans
**
** The reader's spans do not fit; the mover runs on.
*/
void stage3_overflow_spans(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage3_load_mover(h);
    stage3_state_reader(&a);
    a.cfg.spans = 16;
    host_load(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}
