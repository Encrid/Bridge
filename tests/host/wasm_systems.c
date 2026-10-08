/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/wasm_systems.c -- cases that load one wasm module and run it
** for some frames.
*/

#include "cases.h"

static void run_one(host_t *h, const char *identity, const char *file,
    int frames)
{
    if (host_open(h))
        return;
    test_load_wasm(h, identity, file);
    host_run_frames_unsaid(h->b, frames);
    host_close(h);
}

/*
** test_order_own
**
** Two systems of one module that conflict: they run in the order they were
** registered in.
*/
void test_order_own(host_t *h)
{
    run_one(h, "game:twostep@0.1.0", "twostep", 1);
}

/*
** test_chain
**
** An observer's attach triggers the next observer, one hop a barrier.
*/
void test_chain(host_t *h)
{
    run_one(h, "game:chain@0.1.0", "chain", 3);
}

/*
** test_tell_events
**
** Enter and attach, then attach, then leave.
*/
void test_tell_events(host_t *h)
{
    run_one(h, "game:events@0.1.0", "events", 3);
}

/*
** test_nest
**
** A component whose fields are other declared types, in the build the case
** is named after: with them declared, or with one left out.
*/
void test_nest(host_t *h)
{
    run_one(h, "game:nested@0.1.0", h->arg, 1);
}
