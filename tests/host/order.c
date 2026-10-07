/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/order.c -- two systems that conflict in one phase:
** the mover adds 1 and the halver halves, and which runs first is the sum.
**
** schedule.md, *Order within a phase is never implicit*
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
    host_run_frames_unsaid(h->b, 1);
    host_close(h);
}

/*
** test_order
**
** The deployment orders the mover before the halver.
*/
void test_order(host_t *h)
{
    h->cfg.order = &TEST_PAIRS[0];
    h->cfg.order_count = 1;
    load_three(h);
}

/*
** test_reorder
**
** The deployment orders the halver before the mover.
*/
void test_reorder(host_t *h)
{
    h->cfg.order = &TEST_PAIRS[1];
    h->cfg.order_count = 1;
    load_three(h);
}

/*
** test_leave_unordered
**
** Nothing orders them: the second to load is refused.
*/
void test_leave_unordered(host_t *h)
{
    load_three(h);
}

/*
** test_order_in_a_cycle
**
** The deployment orders each before the other.
*/
void test_order_in_a_cycle(host_t *h)
{
    h->cfg.order = TEST_PAIRS;
    h->cfg.order_count = 2;
    host_open_only(h);
}
