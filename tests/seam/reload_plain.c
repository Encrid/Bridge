/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "cases.h"

static void state_ticker(const host_t *h, bridge_artifact_t *a,
    const char *identity, const char *path)
{
    test_state_built_minting(a, identity, path);
    a->cfg.hot_reload = BRIDGE_TRUE;
    a->cfg.reloadable = BRIDGE_TRUE;
    if (!strcmp(h->arg, "nreloadfailstop"))
        a->cfg.on_error = BRIDGE_FAIL;
}

static void reload_to(host_t *h, const char *identity, const char *path)
{
    bridge_artifact_t a;

    state_ticker(h, &a, identity, path);
    host_reload(h, &a);
    host_run_frames(h, 2);
    test_say_modules(h);
}

static void end_failed(host_t *h)
{
    if (strcmp(h->arg, "nreloadfailunload")) {
        reload_to(h, "game:ticker@0.3.0", STEPPER2_PATH);
        return;
    }
    host_unload(h, "game:ticker@0.1.0");
    test_say_modules(h);
}

void test_reload_plain(host_t *h)
{
    bool works = !strcmp(h->arg, "nreload");
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_load_guarded_minting(h, "game:twostep@0.1.0", "twostep");
    state_ticker(h, &a, "game:ticker@0.1.0", STEPPER1_PATH);
    host_load(h, &a);
    host_run_frames(h, 2);
    reload_to(h, "game:ticker@0.2.0", works ? STEPPER2_PATH : STEPPER3_PATH);
    if (!works)
        end_failed(h);
    host_close(h);
}

void test_reload_plain_loss(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    state_ticker(h, &a, "game:ticker@0.1.0", STEPPER1_PATH);
    host_load(h, &a);
    host_run_frames(h, 2);
    state_ticker(h, &a, "game:ticker@0.2.0", STEPPER2_PATH);
    a.cfg.losses = TEST_TICK_LOSS;
    a.cfg.loss_count = 1;
    host_reload(h, &a);
    test_say_modules(h);
    host_close(h);
}
