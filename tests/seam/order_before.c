/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

static const bridge_order_t TEST_PAIR_AGAINST_BEFORE[1] = {
    { "game:twostep:again", "game:twostep:twice" }
};

static const bridge_order_t TEST_PAIR_BESIDE_BEFORE[1] = {
    { "game:twostep:twice", "game:twostep:more" }
};

void test_order_pair_against_before(host_t *h)
{
    h->cfg.order = TEST_PAIR_AGAINST_BEFORE;
    h->cfg.order_count = 1;
    if (host_open(h))
        return;
    test_load_guarded_minting(h, "game:twostep@0.1.0", "twostep_third");
    host_run_frames_unsaid(h->w, 1);
    host_close(h);
}

void test_order_pair_beside_before(host_t *h)
{
    h->cfg.order = TEST_PAIR_BESIDE_BEFORE;
    h->cfg.order_count = 1;
    test_run_guarded(h, "game:twostep@0.1.0", h->arg, 1);
}

void test_reload_into_cycle(host_t *h)
{
    bridge_artifact_t a;

    h->cfg.order = TEST_PAIR_AGAINST_BEFORE;
    h->cfg.order_count = 1;
    if (host_open(h))
        return;
    test_state_guarded(&a, "game:twostep@0.1.0", "twostep_reloadable");
    a.cfg.reloadable = BRIDGE_TRUE;
    a.cfg.hot_reload = BRIDGE_TRUE;
    host_load(h, &a);
    test_state_guarded(&a, "game:twostep@0.2.0", h->arg);
    a.cfg.reloadable = BRIDGE_TRUE;
    a.cfg.hot_reload = BRIDGE_TRUE;
    host_reload(h, &a);
    test_say_modules(h);
    host_close(h);
}
