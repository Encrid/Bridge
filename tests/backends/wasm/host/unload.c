/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "cases.h"

static const char *const TEST_ROSTER[] = {
    "game:Position",
    "game:Roster"
};

void test_fail_unlimited(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_wasm_minting(&a, "test:flaky@0.1.0", "flaky-always");
    a.cfg.spawns = BRIDGE_NONE;
    a.cfg.max_system_failures = BRIDGE_NO_LIMIT;
    host_load(h, &a);
    host_run_frames(h, 8);
    test_say_modules(h);
    host_close(h);
}

static void load_keeper(host_t *h)
{
    char file[32];
    bridge_artifact_t a;

    snprintf(file, sizeof(file), "keeper-%s", h->arg);
    test_load_wasm_minting(h, "game:keeper@0.1.0", file);
    test_state_wasm_minting(&a, "view:counter@0.1.0", "counter");
    a.cfg.read = TEST_ROSTER;
    a.cfg.read_count = 2;
    a.cfg.spawns = BRIDGE_NONE;
    host_load(h, &a);
}

void test_unload_keeper(host_t *h)
{
    if (host_open(h))
        return;
    load_keeper(h);
    host_run_frames(h, 1);
    host_unload(h, "game:keeper@0.1.0");
    host_run_frames(h, 1);
    test_load_spawnless(h, "game:retype@0.1.0", "retype-private");
    test_say_modules(h);
    host_close(h);
}

void test_unload_declarer(host_t *h)
{
    if (host_open(h))
        return;
    test_load_spawnless(h, "game:typer@0.1.0", "typer");
    test_load_wasm_minting(h, "game:user@0.1.0", "user");
    host_unload(h, "game:typer@0.1.0");
    host_run_frames(h, 1);
    test_load_spawnless(h, "game:retype@0.1.0", "retype-vec2");
    host_unload(h, "game:user@0.1.0");
    test_load_spawnless(h, "game:retype@0.1.0", "retype-vec2");
    host_close(h);
}

void test_unload_nobody(host_t *h)
{
    if (host_open(h))
        return;
    host_unload(h, "game:nobody@0.1.0");
    host_close(h);
}
