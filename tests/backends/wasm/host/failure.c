/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

void test_skip_init(host_t *h)
{
    if (host_open(h))
        return;
    test_load_wasm_minting(h, "game:twostep@0.1.0", "twostep");
    test_load_wasm_minting(h, "game:badobs@0.1.0", "badobs");
    host_run_frames(h, 1);
    test_say_modules(h);
    host_close(h);
}

void test_refuse_load(host_t *h)
{
    if (host_open(h))
        return;
    test_load_wasm_minting(h, "game:badterm@0.1.0", "badterm");
    test_load_wasm_minting(h, "game:fp@0.1.0", "fp");
    test_say_modules(h);
    host_close(h);
}

static void fail_always(host_t *h, enum bridge_on_error on_error)
{
    bridge_artifact_t a;

    test_load_wasm_minting(h, "game:twostep@0.1.0", "twostep");
    test_state_wasm_minting(&a, "test:flaky@0.1.0", "flaky-always");
    a.cfg.spawns = BRIDGE_NONE;
    a.cfg.on_error = on_error;
    host_load(h, &a);
    host_run_frames(h, 5);
    test_say_modules(h);
}

void test_reach_budget(host_t *h)
{
    if (host_open(h))
        return;
    fail_always(h, BRIDGE_SKIP);
    host_close(h);
}

void test_stop_by_failing(host_t *h)
{
    if (host_open(h))
        return;
    fail_always(h, BRIDGE_FAIL);
    host_unload(h, "game:twostep@0.1.0");
    host_close(h);
}
