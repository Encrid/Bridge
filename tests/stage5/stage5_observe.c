/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage5/stage5_observe.c -- observers: a native attach seen by a
** wasm observer, and two observers of one change, in two modules, one of
** them writing what the other reads. The barrier is a phase like any other
** to the conflict graph.
*/

#include "stage5.h"

static const bridge_order_t STAGE5_SEEN_FIRST = {
    "view:watcher:seen",
    "view:fixer:fix"
};

static const bridge_order_t STAGE5_FIX_FIRST = {
    "view:fixer:fix",
    "view:watcher:seen"
};

/*
** stage5_observe
**
** A native attach, a wasm observer.
*/
void stage5_observe(host_t *h)
{
    if (host_open(h))
        return;
    stage5_load_native(h, "game:setter@0.1.0", NSETTER_PATH, 8);
    stage5_load_watcher(h);
    host_run_frames_unsaid(h->b, 2);
    host_close(h);
}

/*
** stage5_conflict_at_barrier
**
** Two observers of one change, one writing what the other reads, and
** nothing ordering them: the second to load is refused.
*/
void stage5_conflict_at_barrier(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage5_load_native(h, "game:setter@0.1.0", NSETTER_PATH, 8);
    stage5_load_watcher(h);
    stage5_state_wasm(&a, "view:fixer@0.1.0", "fixer");
    a.cfg.write = &STAGE5_GAME[1];
    a.cfg.write_count = 1;
    host_load(h, &a);
    host_run_frames_unsaid(h->b, 1);
    host_close(h);
}

/*
** stage5_order_observers
**
** The same two, with the watcher ordered first.
*/
void stage5_order_observers(host_t *h)
{
    h->cfg.order = &STAGE5_SEEN_FIRST;
    h->cfg.order_count = 1;
    stage5_conflict_at_barrier(h);
}

/*
** stage5_reorder_observers
**
** The same two, with the fixer ordered first.
*/
void stage5_reorder_observers(host_t *h)
{
    h->cfg.order = &STAGE5_FIX_FIRST;
    h->cfg.order_count = 1;
    stage5_conflict_at_barrier(h);
}

/*
** stage5_take_reserved_name
**
** An observer that takes a name that is a lifecycle export's.
*/
void stage5_take_reserved_name(host_t *h)
{
    if (host_open(h))
        return;
    stage5_load_wasm(h, "game:badobs@0.1.0", "badobs");
    host_close(h);
}
