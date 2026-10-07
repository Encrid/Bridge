/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/reload.c -- the ticker reloaded: into a build that
** counts by ten, one with another component layout, one whose reload
** fails, one that no longer declares what it did, the native one; with
** more ids than the old instance held ready; and where the deployment says
** it may not be.
**
** lifetime.md, *When a component layout changed*
*/

#include <string.h>

#include "cases.h"

static const test_reload_t TEST_RELOADS[] = {
    { "reload", "ticker-ten", BRIDGE_TRUE, 32, nullptr },
    { "reloadexact", "ticker-ten", BRIDGE_TRUE, 32, nullptr },
    { "reloadexactfail", "ticker-fail", BRIDGE_TRUE, 32, nullptr },
    { "reloadspawns", "ticker-ten", BRIDGE_TRUE, 4000, nullptr },
    { "reloadlayout", "ticker-wide", BRIDGE_TRUE, 32, nullptr },
    { "reloadfail", "ticker-fail", BRIDGE_TRUE, 32, nullptr },
    { "reloadmove", nullptr, BRIDGE_TRUE, 32, nullptr },
    { "reloaddrop", "ticker-noextra", BRIDGE_TRUE, 32, "retype-extra" },
    { "maynot", "ticker-ten", BRIDGE_FALSE, 32, nullptr }
};

static const test_reload_t *find_reload(const char *name)
{
    const test_reload_t *r = TEST_RELOADS;

    while (strcmp(r->name, name))
        r++;
    return (r);
}

/*
** state_next
**
** States what the ticker is reloaded into. The new instance may state more
** ids than the old held ready: its own block of them is as long as it
** states, and takes the old one's in.
*/
static void state_next(bridge_artifact_t *a, const test_reload_t *r)
{
    if (r->file)
        test_state_ticker(a, "game:ticker@0.2.0", r->file);
    else
        test_state_native_minting(a, "game:ticker@0.2.0", NTICKER1_PATH);
    a->cfg.hot_reload = BRIDGE_TRUE;
    a->cfg.reloadable = BRIDGE_TRUE;
    a->cfg.spawns = r->spawns;
}

/*
** test_reload_ticker
**
** Loads the ticker and reloads it as the case is named. Two of the cases
** have room for one system: the ticker has one, and a reload still has
** room. What the new instance no longer declares went as an unload would
** take it, so its name is free for the module loaded afterwards.
*/
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
    host_reload(h, &a);
    host_run_frames(h, 2);
    if (r->then)
        test_load_spawnless(h, "game:retype@0.1.0", r->then);
    test_say_modules(h);
    host_close(h);
}

/*
** test_reload_nothing
**
** A reload of a module that is not loaded.
*/
void test_reload_nothing(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_ticker(&a, "game:ticker@0.2.0", "ticker-ten");
    host_reload(h, &a);
    host_close(h);
}

/*
** test_revive
**
** A stopped module, reloaded into one that works.
*/
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
