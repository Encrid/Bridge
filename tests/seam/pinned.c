/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "cases.h"

test_counts_t *test_get_counts(const host_t *h)
{
    const test_listener_t *l = h->ud;

    return (l->counts);
}

static void load_pinned(host_t *h)
{
    bridge_artifact_t a;

    host_state_built(&a, "test:pinned@0.1.0", PINNED_PATH);
    a.cfg.pinned = BRIDGE_TRUE;
    host_load(h, &a);
}

void test_pin(host_t *h)
{
    const test_counts_t *c = test_get_counts(h);

    h->cfg.threads = 4;
    if (host_open(h))
        return;
    test_load_built(h, "test:sleep1@0.1.0", SLEEP1_PATH, BRIDGE_NONE);
    test_load_built(h, "test:sleep2@0.1.0", SLEEP2_PATH, BRIDGE_NONE);
    load_pinned(h);
    host_run_frames_unsaid(h->w, 3);
    printf("the others used %u thread(s)\n", test_count_threads(c));
    printf("pinned: %u of its %u calls were on the thread that loaded it\n",
        c->pins_here, c->pins);
    host_close(h);
}

static int call_elsewhere(bridge_world_t *w, int frames)
{
    test_elsewhere_t e = {};

    test_start_elsewhere(&e, w, frames);
    test_join_elsewhere(&e);
    return (e.result);
}

void test_call_from_wrong_thread(host_t *h)
{
    h->cfg.threads = 4;
    if (host_open(h))
        return;
    load_pinned(h);
    printf("from another thread: %s\n",
        call_elsewhere(h->w, 0) ? "refused" : "ran");
    printf("destroy from another thread: %s\n",
        call_elsewhere(h->w, -1) ? "refused" : "done");
    printf("destroy from the thread that loaded it: %s\n",
        bridge_destroy(h->w) ? "refused" : "done");
    printf("alive\n");
}
