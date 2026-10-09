/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/pinned.c -- a pinned module: its code runs on the
** thread that loaded it, or not at all.
**
** threads.md, *Threads*
*/

#include <stdio.h>

#include "cases.h"

/*
** test_get_counts
**
** Answers what the host's sink counts.
*/
test_counts_t *test_get_counts(const host_t *h)
{
    const test_listener_t *l = h->ud;

    return (l->counts);
}

static void load_pinned(host_t *h)
{
    bridge_artifact_t a;

    host_state_built(&a, "test:pinned@0.1.0", "pinned");
    a.cfg.pinned = BRIDGE_TRUE;
    host_load(h, &a);
}

/*
** test_pin
**
** Three systems, pinned, among others. The naps run beside it, on
** whichever threads: its own lines are the ones counted.
*/
void test_pin(host_t *h)
{
    const test_counts_t *c = test_get_counts(h);

    h->cfg.threads = 4;
    if (host_open(h))
        return;
    test_load_built(h, "test:sleep1@0.1.0", "sleep1", BRIDGE_NONE);
    test_load_built(h, "test:sleep2@0.1.0", "sleep2", BRIDGE_NONE);
    load_pinned(h);
    host_run_frames_unsaid(h->b, 3);
    printf("the others used %u thread(s)\n", test_count_threads(c));
    printf("pinned: %u of its %u calls were on the thread that loaded it\n",
        c->pins_here, c->pins);
    host_close(h);
}

/*
** call_elsewhere
**
** Makes a call from another thread, and answers what it returned.
*/
static int call_elsewhere(bridge_t *b, int frames)
{
    test_elsewhere_t e = {};

    test_start_elsewhere(&e, b, frames);
    test_join_elsewhere(&e);
    return (e.result);
}

/*
** test_call_from_wrong_thread
**
** A phase run from a thread that did not load the pinned module is refused,
** and so is a destroy from there, leaving the deck to be destroyed here.
*/
void test_call_from_wrong_thread(host_t *h)
{
    h->cfg.threads = 4;
    if (host_open(h))
        return;
    load_pinned(h);
    printf("from another thread: %s\n",
        call_elsewhere(h->b, 0) ? "refused" : "ran");
    printf("destroy from another thread: %s\n",
        call_elsewhere(h->b, -1) ? "refused" : "done");
    printf("destroy from the thread that loaded it: %s\n",
        bridge_destroy(h->b) ? "refused" : "done");
    printf("alive\n");
}
