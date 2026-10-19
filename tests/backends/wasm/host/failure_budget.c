/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

void test_give_up(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_load_wasm_minting(h, "game:twostep@0.1.0", "twostep");
    test_state_wasm_minting(&a, "test:flaky@0.1.0", "flaky-always");
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
    test_state_wasm_minting(&a, "test:flaky@0.1.0", file);
    a.cfg.spawns = BRIDGE_NONE;
    a.cfg.max_system_failures = limit;
    host_load(h, &a);
    host_run_frames(h, frames);
    test_say_modules(h);
    host_close(h);
}

void test_forgive(host_t *h)
{
    run_flaky(h, "flaky-third", 12, 3);
}

void test_forgive_too_little(host_t *h)
{
    run_flaky(h, "flaky-twothirds", 12, 3);
}

void test_fail_beside_clean(host_t *h)
{
    run_flaky(h, "flaky-calm", 5, 3);
}
