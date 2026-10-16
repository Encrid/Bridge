/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "cases.h"

static void load_hang(host_t *h)
{
    bridge_artifact_t a;

    test_state_guarded_minting(&a, "test:hang@0.1.0", "hang");
    a.cfg.spawns = BRIDGE_NONE;
    a.cfg.shutdown_timeout = 100 * HOST_MS;
    host_load(h, &a);
}

void test_destroy_loaded(host_t *h)
{
    if (host_open(h))
        return;
    test_load_guarded_minting(h, "game:ticker@0.1.0", "ticker");
    load_hang(h);
    host_run_frames(h, 1);
    host_close(h);
}

void test_time_out_shutdown(host_t *h)
{
    if (host_open(h))
        return;
    load_hang(h);
    host_unload(h, "test:hang@0.1.0");
    test_say_modules(h);
    host_close(h);
}

void test_take_slot(host_t *h)
{
    h->cfg.budget.modules = 1;
    if (host_open(h))
        return;
    test_load_guarded_minting(h, "game:twostep@0.1.0", "twostep");
    test_load_guarded_minting(h, "game:ticker@0.1.0", "ticker");
    host_unload(h, "game:twostep@0.1.0");
    test_load_guarded_minting(h, "game:ticker@0.1.0", "ticker");
    test_say_modules(h);
    host_close(h);
}

void test_state_plain_timeout(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_plain_minting(&a, "game:ticker@0.1.0", "no-such-library");
    a.cfg.shutdown_timeout = 100 * HOST_MS;
    host_load(h, &a);
    host_close(h);
}
