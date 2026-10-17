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
