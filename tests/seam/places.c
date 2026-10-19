/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

static void load_places(host_t *h, const char *first)
{
    if (host_open(h))
        return;
    test_load_guarded(h, "game:places@0.1.0", first);
    test_load_guarded(h, "game:other@0.1.0", "places");
    host_run_frames_unsaid(h->b, 1);
    host_close(h);
}

void test_tell_places(host_t *h)
{
    test_run_guarded(h, "game:places@0.1.0", "places", 1);
}

void test_place_before(host_t *h)
{
    load_places(h, "places_before");
}

void test_place_two(host_t *h)
{
    load_places(h, "places");
}
