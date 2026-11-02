/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "cases.h"

static const bridge_order_t TEST_PAIR_OF_PLACES[1] = {
    { "game:other:second", "game:places:first" }
};

static const bridge_order_t TEST_PAIR_IN_PLACES[1] = {
    { "game:places:second", "game:places:first" }
};

static void put_order(host_t *h, const bridge_order_t *order)
{
    h->cfg.order = order;
    h->cfg.order_count = 1;
}

void test_tell_places(host_t *h)
{
    if (!strcmp(h->arg, "reordered"))
        put_order(h, TEST_PAIR_IN_PLACES);
    test_run_guarded(h, "game:places@0.1.0", "places", 1);
}

void test_place_two(host_t *h)
{
    char file[16] = "";
    char order[16] = "";
    int count = sscanf(h->arg, "%15s %15s", file, order);

    if (count == 2 && !strcmp(order, "paired"))
        put_order(h, TEST_PAIR_OF_PLACES);
    if (host_open(h))
        return;
    test_load_guarded(h, "game:places@0.1.0", file);
    test_load_guarded(h, "game:other@0.1.0", "places");
    host_run_frames_unsaid(h->w, 1);
    host_close(h);
}

void test_place_after_refused_group(host_t *h)
{
    bridge_artifact_t a[2];

    put_order(h, TEST_PAIR_OF_PLACES);
    if (host_open(h))
        return;
    test_load_guarded(h, "game:places@0.1.0", "places");
    test_load_guarded(h, "game:other@0.1.0", "places");
    group_state(&a[0], "game:one@0.1.0", "share1");
    group_state(&a[1], "game:two@0.1.0", "share1");
    host_load(h, &a[0]);
    host_load(h, &a[1]);
    host_run_frames_unsaid(h->w, 1);
    group_state(&a[0], "game:one@0.2.0", h->arg);
    group_state(&a[1], "game:two@0.2.0", h->arg);
    host_reload_many(h, a, 2);
    host_run_frames_unsaid(h->w, 1);
    host_close(h);
}
