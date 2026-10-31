/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "cases.h"

void test_place_after_stop(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_guarded_minting(&a, "test:flaky@0.1.0", "flaky-always");
    a.cfg.spawns = BRIDGE_NONE;
    a.cfg.on_error = BRIDGE_SKIP;
    host_load(h, &a);
    test_load_guarded(h, "game:places@0.1.0", "places");
    host_run_frames_unsaid(h->w, 5);
    host_close(h);
}

void test_place_after_reload(host_t *h)
{
    bridge_artifact_t a[2];

    if (host_open(h))
        return;
    group_state(&a[0], "game:one@0.1.0", "share1");
    group_state(&a[1], "game:two@0.1.0", "share1");
    host_load(h, &a[0]);
    host_load(h, &a[1]);
    host_run_frames_unsaid(h->w, 1);
    group_state(&a[0], "game:one@0.2.0", "share1");
    host_reload(h, &a[0]);
    host_run_frames_unsaid(h->w, 1);
    host_close(h);
}

void test_place_after_refused_load(host_t *h)
{
    bridge_artifact_t a[2];

    if (host_open(h))
        return;
    test_load_guarded(h, "game:places@0.1.0", "places");
    test_load_guarded(h, "game:other@0.1.0", "places");
    group_state(&a[0], "game:one@0.1.0", "sharew");
    host_load(h, &a[0]);
    group_state(&a[1], "game:two@0.1.0", "sharew");
    host_load(h, &a[1]);
    host_run_frames_unsaid(h->w, 1);
    host_close(h);
}

void test_heard_triggered(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    group_state(&a, "game:twoears@0.1.0",
        strcmp(h->arg, "both") ? "twoears" : "twoears_both");
    host_load(h, &a);
    if (!strcmp(h->arg, "two")) {
        group_state(&a, "game:ears@0.1.0", "twoears_ears");
        host_load(h, &a);
    }
    host_run_frames_unsaid(h->w, 1);
    host_close(h);
}
