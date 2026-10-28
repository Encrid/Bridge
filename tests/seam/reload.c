/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "cases.h"

static const test_reload_t TEST_RELOADS[] = {
    { "reload", "ticker-ten", BRIDGE_TRUE, 32, nullptr },
    { "reloadexact", "ticker-ten", BRIDGE_TRUE, 32, nullptr },
    { "reloadexactfail", "ticker-fail", BRIDGE_TRUE, 32, nullptr },
    { "reloadspawns", "ticker-ten", BRIDGE_TRUE, 4000, nullptr },
    { "reloadfewer", "ticker-ten", BRIDGE_TRUE, 4, nullptr },
    { "reloadlayout", "ticker-wide", BRIDGE_TRUE, 32, nullptr },
    { "reloadfail", "ticker-fail", BRIDGE_TRUE, 32, nullptr },
    { "reloadmove", nullptr, BRIDGE_TRUE, 32, nullptr },
    { "reloaddrop", "ticker-noextra", BRIDGE_TRUE, 32, "retype-extra" },
    { "reloadkeep", "ticker-keep", BRIDGE_TRUE, 32, nullptr },
    { "reloadloss", "ticker-ten", BRIDGE_TRUE, 32, nullptr },
    { "maynot", "ticker-ten", BRIDGE_FALSE, 32, nullptr }
};

static const test_reload_t *find_reload(const char *name)
{
    const test_reload_t *r = TEST_RELOADS;

    while (strcmp(r->name, name))
        r++;
    return (r);
}

static void state_next(bridge_artifact_t *a, const test_reload_t *r)
{
    if (r->file)
        test_state_ticker(a, "game:ticker@0.2.0", r->file);
    else
        test_state_built_minting(a, "game:ticker@0.2.0", STEPPER1_PATH);
    a->cfg.hot_reload = BRIDGE_TRUE;
    a->cfg.reloadable = BRIDGE_TRUE;
    a->cfg.spawns = r->spawns;
}

void test_reload_ticker(host_t *h)
{
    const test_reload_t *r = find_reload(h->arg);
    bridge_artifact_t a;

    if (!strncmp(h->arg, "reloadexact", 11))
        h->cfg.budget.systems = 1;
    if (host_open(h))
        return;
    test_state_ticker(&a, "game:ticker@0.1.0", "ticker");
    a.cfg.hot_reload = r->may;
    host_load(h, &a);
    host_run_frames(h, 2);
    state_next(&a, r);
    if (!strcmp(h->arg, "reloadloss")) {
        a.cfg.losses = TEST_TICK_LOSS;
        a.cfg.loss_count = 1;
    }
    host_reload(h, &a);
    host_run_frames(h, 2);
    if (r->then)
        test_load_spawnless(h, "game:retype@0.1.0", r->then);
    test_say_modules(h);
    host_close(h);
}

void test_reload_tag(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_ticker(&a, "game:ticker@0.1.0", "ticker-tag");
    host_load(h, &a);
    host_run_frames(h, 2);
    test_state_ticker(&a, "game:ticker@0.2.0", "ticker-tag");
    host_reload(h, &a);
    host_run_frames(h, 2);
    test_say_modules(h);
    host_close(h);
}

void test_reload_nothing(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_ticker(&a, "game:ticker@0.2.0", "ticker-ten");
    host_reload(h, &a);
    host_close(h);
}

void test_revive(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_ticker(&a, "game:ticker@0.1.0", "ticker-trip");
    host_load(h, &a);
    host_run_frames(h, 4);
    test_say_modules(h);
    test_state_ticker(&a, "game:ticker@0.2.0", "ticker-ten");
    host_reload(h, &a);
    host_run_frames(h, 2);
    test_say_modules(h);
    host_close(h);
}
