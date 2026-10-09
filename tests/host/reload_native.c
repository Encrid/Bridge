/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/host/reload_native.c -- a native module reloaded: the old
** instance is shut down before the new library is opened, so a reload that
** fails leaves no code to keep.
**
** failure.md, *When something fails*
*/

#include <string.h>

#include "cases.h"

/*
** state_ticker
**
** States a native build of the ticker that may be reloaded, and is
** essential in the case named for that.
*/
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

/*
** end_failed
**
** What follows a reload that failed. In one case the verb that discards: a
** module with no code left can still be unloaded. In the others its rows
** were kept: an artifact that works picks up where the count was.
*/
static void end_failed(host_t *h)
{
    if (strcmp(h->arg, "nreloadfailunload")) {
        reload_to(h, "game:ticker@0.3.0", "stepper2");
        return;
    }
    host_unload(h, "game:ticker@0.1.0");
    test_say_modules(h);
}

/*
** test_reload_native
**
** Loads the native ticker beside a wasm module and reloads it: into a
** library that works, or into one whose reload fails.
*/
void test_reload_native(host_t *h)
{
    bool works = !strcmp(h->arg, "nreload");
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_load_wasm_minting(h, "game:twostep@0.1.0", "twostep");
    state_ticker(h, &a, "game:ticker@0.1.0", "stepper1");
    host_load(h, &a);
    host_run_frames(h, 2);
    reload_to(h, "game:ticker@0.2.0", works ? "stepper2" : "stepper3");
    if (!works)
        end_failed(h);
    host_close(h);
}
