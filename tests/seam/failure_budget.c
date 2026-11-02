/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "cases.h"

void test_give_up(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_load_guarded_minting(h, "game:twostep@0.1.0", "twostep");
    test_state_guarded_minting(&a, "test:flaky@0.1.0", h->arg);
    a.cfg.spawns = BRIDGE_NONE;
    a.cfg.on_error = BRIDGE_FAIL;
    host_load(h, &a);
    host_run_frames(h, 5);
    test_say_modules(h);
    host_unload(h, "game:twostep@0.1.0");
    host_unload(h, "test:flaky@0.1.0");
    host_run_frames(h, 1);
    test_say_modules(h);
    host_close(h);
}

static void run_flaky(host_t *h, const char *file, int frames, int64_t limit)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_guarded_minting(&a, "test:flaky@0.1.0", file);
    a.cfg.spawns = BRIDGE_NONE;
    a.cfg.max_system_failures = limit;
    host_load(h, &a);
    host_run_frames(h, frames);
    test_say_modules(h);
    host_close(h);
}

void test_forgive(host_t *h)
{
    char file[32] = "";
    int frames = 0;
    long long limit = 0;

    (void)sscanf(h->arg, "%31s %d %lld", file, &frames, &limit);
    run_flaky(h, file, frames, limit);
}
