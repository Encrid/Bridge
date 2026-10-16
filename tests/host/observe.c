/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
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

void test_observe(host_t *h)
{
    if (host_open(h))
        return;
    test_load_built(h, "game:setter@0.1.0", SETTER_PATH, 8);
    test_load_watcher(h);
    host_run_frames_unsaid(h->b, 2);
    host_close(h);
}

void test_conflict_at_barrier(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_load_built(h, "game:setter@0.1.0", SETTER_PATH, 8);
    test_load_watcher(h);
    test_state_guarded(&a, "view:fixer@0.1.0", "fixer");
    a.cfg.write = &TEST_GAME[1];
    a.cfg.write_count = 1;
    host_load(h, &a);
    host_run_frames_unsaid(h->b, 1);
    host_close(h);
}

void test_order_observers(host_t *h)
{
    h->cfg.order = &TEST_SEEN_FIRST;
    h->cfg.order_count = 1;
    test_conflict_at_barrier(h);
}

void test_reorder_observers(host_t *h)
{
    h->cfg.order = &TEST_FIX_FIRST;
    h->cfg.order_count = 1;
    test_conflict_at_barrier(h);
}

void test_take_reserved_name(host_t *h)
{
    if (host_open(h))
        return;
    test_load_guarded(h, "game:badobs@0.1.0", "badobs");
    host_close(h);
}
