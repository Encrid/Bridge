/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

static void run_one(host_t *h, const char *identity, const char *file,
    int frames)
{
    if (host_open(h))
        return;
    test_load_wasm(h, identity, file);
    host_run_frames_unsaid(h->w, frames);
    host_close(h);
}

void test_order_own(host_t *h)
{
    run_one(h, "game:twostep@0.1.0", "twostep", 1);
}

void test_chain(host_t *h)
{
    run_one(h, "game:chain@0.1.0", "chain", 3);
}

void test_tell_events(host_t *h)
{
    run_one(h, "game:events@0.1.0", "events", 3);
}

void test_nest(host_t *h)
{
    run_one(h, "game:nested@0.1.0", h->arg, 1);
}
