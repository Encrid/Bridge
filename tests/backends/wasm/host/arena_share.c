/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "cases.h"

static int64_t get_next_share(const char *what)
{
    if (!strcmp(what, "sharesame"))
        return (16);
    if (!strcmp(what, "shareless"))
        return (8);
    if (!strcmp(what, "sharebig"))
        return (1 << 20);
    return (64);
}

static void load_namer(host_t *h)
{
    bridge_artifact_t a;

    test_state_wasm_minting(&a, "game:namer@0.1.0", "namer");
    a.cfg.persistent = 64;
    host_load(h, &a);
}

void test_grow_share(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_ticker(&a, "game:ticker@0.1.0", "ticker");
    a.cfg.persistent = 16;
    host_load(h, &a);
    if (!strcmp(h->arg, "sharechain"))
        load_namer(h);
    host_run_frames(h, 2);
    test_state_ticker(&a, "game:ticker@0.2.0", "ticker-alloc");
    a.cfg.persistent = get_next_share(h->arg);
    host_reload(h, &a);
    host_run_frames(h, 2);
    test_say_modules(h);
    host_close(h);
}

void test_use_arena(host_t *h)
{
    bool kept = !strcmp(h->arg, "arenakept");
    bridge_artifact_t a;

    h->cfg.budget.persistent = 64;
    if (host_open(h))
        return;
    load_namer(h);
    if (kept)
        test_load_wasm_minting(h, "game:reader@0.1.0", "namer-reader");
    host_run_frames(h, 1);
    host_unload(h, "game:namer@0.1.0");
    host_run_frames(h, 1);
    test_state_wasm_minting(&a, "game:ticker@0.1.0", "ticker");
    a.cfg.persistent = 64;
    host_load(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}
