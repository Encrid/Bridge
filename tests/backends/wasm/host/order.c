/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

static const bridge_order_t TEST_PAIRS[2] = {
    { "game:mover:move", "view:halver:halve" },
    { "view:halver:halve", "game:mover:move" }
};

static void load_three(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_load_wasm(h, "game:mover@0.1.0", "mover");
    test_state_wasm(&a, "view:halver@0.1.0", "halver");
    a.cfg.write = TEST_GAME;
    a.cfg.write_count = 1;
    host_load(h, &a);
    test_load_game_reader(h);
    host_run_frames_unsaid(h->w, 1);
    host_close(h);
}

void test_order(host_t *h)
{
    h->cfg.order = &TEST_PAIRS[0];
    h->cfg.order_count = 1;
    load_three(h);
}

void test_reorder(host_t *h)
{
    h->cfg.order = &TEST_PAIRS[1];
    h->cfg.order_count = 1;
    load_three(h);
}

void test_leave_unordered(host_t *h)
{
    load_three(h);
}

void test_order_in_a_cycle(host_t *h)
{
    h->cfg.order = TEST_PAIRS;
    h->cfg.order_count = 2;
    host_open_only(h);
}
