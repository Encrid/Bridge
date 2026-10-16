/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

static const bridge_order_t TEST_CHAIN_PAIRS[2] = {
    { "game:mover:move", "view:counter:count" },
    { "view:counter:count", "view:halver:halve" }
};

static const bridge_order_t TEST_THROUGH_PAIRS[2] = {
    { "game:mover:move", "game:twostep:twice" },
    { "game:twostep:more", "view:halver:halve" }
};

static const char *const TEST_COUNTER_READS[] = {
    "game:Position",
    "game:Roster"
};

static void load_halver(host_t *h)
{
    bridge_artifact_t a;

    test_state_guarded(&a, "view:halver@0.1.0", "halver");
    a.cfg.write = TEST_GAME;
    a.cfg.write_count = 1;
    host_load(h, &a);
}

static void load_counter(host_t *h)
{
    bridge_artifact_t a;

    test_state_guarded_minting(&a, "view:counter@0.1.0", "counter");
    a.cfg.read = TEST_COUNTER_READS;
    a.cfg.read_count = 2;
    a.cfg.spawns = BRIDGE_NONE;
    host_load(h, &a);
}

void test_order_chain_unloaded(host_t *h)
{
    h->cfg.order = TEST_CHAIN_PAIRS;
    h->cfg.order_count = 2;
    if (host_open(h))
        return;
    test_load_guarded(h, "game:mover@0.1.0", "mover");
    load_counter(h);
    load_halver(h);
    host_unload(h, "view:counter@0.1.0");
    test_load_game_reader(h);
    host_close(h);
}

void test_order_through_a_module(host_t *h)
{
    h->cfg.order = TEST_THROUGH_PAIRS;
    h->cfg.order_count = 2;
    if (host_open(h))
        return;
    test_load_guarded(h, "game:mover@0.1.0", "mover");
    test_load_guarded(h, "game:twostep@0.1.0", "twostep");
    load_halver(h);
    host_unload(h, "game:twostep@0.1.0");
    test_load_game_reader(h);
    host_close(h);
}
