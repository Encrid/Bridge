/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage5/stage5_stated.c -- what a module stated it changes: the
** bytes of its queue, and the components a system attaches.
**
** budgets.md, *Nothing is allocated in a phase*
*/

#include "stage5.h"

static void load_stamp(host_t *h, int64_t changes)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    host_state_native(&a, "game:stamp5@0.1.0", NSTAMP5_PATH);
    a.cfg.spawns = 8;
    a.cfg.changes = changes;
    host_load(h, &a);
    host_close(h);
}

/*
** stage5_queue_too_few
**
** Its init attaches once, which is 32 bytes of queue: stated as 16.
*/
void stage5_queue_too_few(host_t *h)
{
    load_stamp(h, 16);
}

/*
** stage5_queue_none
**
** Its init attaches once: stated as queueing nothing.
*/
void stage5_queue_none(host_t *h)
{
    load_stamp(h, BRIDGE_NONE);
}

/*
** stage5_change_unstated
**
** A system attaches what it did not state among its changes: that
** invocation fails.
*/
void stage5_change_unstated(host_t *h)
{
    if (host_open(h))
        return;
    stage5_load_native(h, "game:stamp@0.1.0", NSTAMPUNSTATED_PATH, 8);
    stage5_load_watcher(h);
    host_run_frames_unsaid(h->b, 2);
    host_close(h);
}

/*
** stage5_hop_threads
**
** Wasm instances on four threads: a store is called from whichever thread
** took its module's job, never from two at once. Fifty frames, and the last
** sum is exact.
*/
void stage5_hop_threads(host_t *h)
{
    h->cfg.threads = 4;
    if (host_open(h))
        return;
    stage5_load_wasm(h, "game:mover@0.1.0", "mover");
    stage5_load_reader(h);
    host_run_frames_unsaid(h->b, 50);
    host_close(h);
}
