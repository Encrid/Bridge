/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

static const bridge_order_t TEST_PAIR_OF_PLACES[1] = {
    { "game:other:second", "game:places:first" }
};

static const bridge_order_t TEST_PAIR_IN_PLACES[1] = {
    { "game:places:second", "game:places:first" }
};

void test_place_paired(host_t *h)
{
    h->cfg.order = TEST_PAIR_OF_PLACES;
    h->cfg.order_count = 1;
    test_place_two(h);
}

void test_place_reordered(host_t *h)
{
    h->cfg.order = TEST_PAIR_IN_PLACES;
    h->cfg.order_count = 1;
    test_tell_places(h);
}

void test_place_after_refused_group(host_t *h)
{
    bridge_artifact_t a[2];

    h->cfg.order = TEST_PAIR_OF_PLACES;
    h->cfg.order_count = 1;
    if (host_open(h))
        return;
    test_load_guarded(h, "game:places@0.1.0", "places");
    test_load_guarded(h, "game:other@0.1.0", "places");
    group_state(&a[0], "game:one@0.1.0", "share1");
    group_state(&a[1], "game:two@0.1.0", "share1");
    host_load(h, &a[0]);
    host_load(h, &a[1]);
    host_run_frames_unsaid(h->w, 1);
    group_state(&a[0], "game:one@0.2.0", "sharew");
    group_state(&a[1], "game:two@0.2.0", "sharew");
    host_reload_many(h, a, 2);
    host_run_frames_unsaid(h->w, 1);
    host_close(h);
}
