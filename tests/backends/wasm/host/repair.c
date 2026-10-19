/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

void test_repair(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_ticker(&a, "game:ticker@0.1.0", "ticker-trip");
    a.cfg.on_error = BRIDGE_FAIL;
    host_load(h, &a);
    host_run_frames(h, 4);
    test_load_wasm_minting(h, "game:twostep@0.1.0", "twostep");
    test_state_ticker(&a, "game:ticker@0.2.0", "ticker-ten");
    a.cfg.on_error = BRIDGE_FAIL;
    host_reload(h, &a);
    host_run_frames(h, 2);
    test_say_modules(h);
    host_close(h);
}

void test_repair_by_load(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_wasm_minting(&a, "game:thing@0.1.0", "badobs");
    a.cfg.on_error = BRIDGE_FAIL;
    host_load(h, &a);
    test_load_wasm_minting(h, "game:other@0.1.0", "twostep");
    test_state_wasm_minting(&a, "game:thing@0.2.0", "twostep");
    a.cfg.on_error = BRIDGE_FAIL;
    host_load(h, &a);
    host_run_frames(h, 1);
    test_say_modules(h);
    host_close(h);
}

void test_allow_not_reloadable(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_wasm_minting(&a, "game:twostep@0.1.0", "twostep");
    a.cfg.hot_reload = BRIDGE_TRUE;
    host_load(h, &a);
    host_close(h);
}

void test_leave_reload_out(host_t *h)
{
    if (host_open(h))
        return;
    test_load_spawnless(h, "test:noreload@0.1.0", "noreload");
    host_close(h);
}
