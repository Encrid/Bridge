/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage5/stage5_pinned.c -- a pinned module: its code runs on the
** thread that loaded it, or not at all.
**
** threads.md, *Threads*
*/

#include <stdio.h>

#include "stage5.h"

/*
** stage5_get_counts
**
** Answers what the host's sink counts.
*/
stage5_counts_t *stage5_get_counts(const host_t *h)
{
    const stage5_listener_t *l = h->ud;

    return (l->counts);
}

static void load_pinned(host_t *h)
{
    bridge_artifact_t a;

    host_state_native(&a, "test:pinned@0.1.0", NPINNED_PATH);
    a.cfg.pinned = BRIDGE_TRUE;
    host_load(h, &a);
}

/*
** stage5_pin
**
** Three systems, pinned, among others. The naps run beside it, on
** whichever threads: its own lines are the ones counted.
*/
void stage5_pin(host_t *h)
{
    const stage5_counts_t *c = stage5_get_counts(h);

    h->cfg.threads = 4;
    if (host_open(h))
        return;
    stage5_load_native(h, "test:sleep1@0.1.0", NSLEEP1_PATH, BRIDGE_NONE);
    stage5_load_native(h, "test:sleep2@0.1.0", NSLEEP2_PATH, BRIDGE_NONE);
    load_pinned(h);
    host_run_frames_unsaid(h->b, 3);
    printf("the others used %u thread(s)\n", stage5_count_threads(c));
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
    stage5_elsewhere_t e = {};

    stage5_start_elsewhere(&e, b, frames);
    stage5_join_elsewhere(&e);
    return (e.result);
}

/*
** stage5_call_from_wrong_thread
**
** A phase run from a thread that did not load the pinned module is refused,
** and so is a destroy from there, leaving the bridge to be destroyed here.
*/
void stage5_call_from_wrong_thread(host_t *h)
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
