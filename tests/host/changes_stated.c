/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/changes_stated.c -- what a module stated it changes: the
** bytes of its queue, and the components a system attaches.
**
** budgets.md, *Nothing is allocated in a phase*
*/

#include "cases.h"

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
** test_queue_too_few
**
** Its init attaches once, which is 32 bytes of queue: stated as 16.
*/
void test_queue_too_few(host_t *h)
{
    load_stamp(h, 16);
}

/*
** test_queue_none
**
** Its init attaches once: stated as queueing nothing.
*/
void test_queue_none(host_t *h)
{
    load_stamp(h, BRIDGE_NONE);
}

/*
** test_change_unstated
**
** A system attaches what it did not state among its changes: that
** invocation fails.
*/
void test_change_unstated(host_t *h)
{
    if (host_open(h))
        return;
    test_load_native(h, "game:stamp@0.1.0", NSTAMPUNSTATED_PATH, 8);
    test_load_watcher(h);
    host_run_frames_unsaid(h->b, 2);
    host_close(h);
}

/*
** test_hop_threads
**
** Wasm instances on four threads: a store is called from whichever thread
** took its module's job, never from two at once. Fifty frames, and the last
** sum is exact.
*/
void test_hop_threads(host_t *h)
{
    h->cfg.threads = 4;
    if (host_open(h))
        return;
    test_load_wasm(h, "game:mover@0.1.0", "mover");
    test_load_game_reader(h);
    host_run_frames_unsaid(h->b, 50);
    host_close(h);
}
