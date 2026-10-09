/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/observe.c -- observers: a native attach seen by a
** wasm observer, and two observers of one change, in two modules, one of
** them writing what the other reads. The barrier is a phase like any other
** to the conflict graph.
*/

#include "cases.h"

static const bridge_order_t TEST_SEEN_FIRST = {
    "view:watcher:seen",
    "view:fixer:fix"
};

static const bridge_order_t TEST_FIX_FIRST = {
    "view:fixer:fix",
    "view:watcher:seen"
};

/*
** test_observe
**
** A native attach, a wasm observer.
*/
void test_observe(host_t *h)
{
    if (host_open(h))
        return;
    test_load_built(h, "game:setter@0.1.0", SETTER_PATH, 8);
    test_load_watcher(h);
    host_run_frames_unsaid(h->b, 2);
    host_close(h);
}

/*
** test_conflict_at_barrier
**
** Two observers of one change, one writing what the other reads, and
** nothing ordering them: the second to load is refused.
*/
void test_conflict_at_barrier(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_load_built(h, "game:setter@0.1.0", SETTER_PATH, 8);
    test_load_watcher(h);
    test_state_wasm(&a, "view:fixer@0.1.0", "fixer");
    a.cfg.write = &TEST_GAME[1];
    a.cfg.write_count = 1;
    host_load(h, &a);
    host_run_frames_unsaid(h->b, 1);
    host_close(h);
}

/*
** test_order_observers
**
** The same two, with the watcher ordered first.
*/
void test_order_observers(host_t *h)
{
    h->cfg.order = &TEST_SEEN_FIRST;
    h->cfg.order_count = 1;
    test_conflict_at_barrier(h);
}

/*
** test_reorder_observers
**
** The same two, with the fixer ordered first.
*/
void test_reorder_observers(host_t *h)
{
    h->cfg.order = &TEST_FIX_FIRST;
    h->cfg.order_count = 1;
    test_conflict_at_barrier(h);
}

/*
** test_take_reserved_name
**
** An observer that takes a name that is a lifecycle export's.
*/
void test_take_reserved_name(host_t *h)
{
    if (host_open(h))
        return;
    test_load_wasm(h, "game:badobs@0.1.0", "badobs");
    host_close(h);
}
