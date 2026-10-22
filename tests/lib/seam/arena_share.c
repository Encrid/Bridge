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

static int64_t get_first_share(const char *what)
{
    return (strcmp(what, "sharefromnone") ? 16 : BRIDGE_NONE);
}

static const char *get_next_file(const char *what)
{
    if (strstr(what, "fail"))
        return ("ticker-fail");
    if (!strcmp(what, "sharechaintwice"))
        return ("ticker-ten");
    return ("ticker-alloc");
}

static void load_namer(host_t *h)
{
    bridge_artifact_t a;

    test_state_guarded_minting(&a, "game:namer@0.1.0", "namer");
    a.cfg.persistent = 64;
    host_load(h, &a);
}

void test_grow_share(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    test_state_ticker(&a, "game:ticker@0.1.0", "ticker");
    a.cfg.persistent = get_first_share(h->arg);
    host_load(h, &a);
    if (!strncmp(h->arg, "sharechain", 10))
        load_namer(h);
    host_run_frames(h, 2);
    test_state_ticker(&a, "game:ticker@0.2.0", get_next_file(h->arg));
    a.cfg.persistent = get_next_share(h->arg);
    host_reload(h, &a);
    if (!strcmp(h->arg, "sharechaintwice")) {
        test_state_ticker(&a, "game:ticker@0.3.0", "ticker-ten");
        a.cfg.persistent = 128;
        host_reload(h, &a);
    }
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
        test_load_guarded_minting(h, "game:reader@0.1.0", "namer-reader");
    host_run_frames(h, 1);
    host_unload(h, "game:namer@0.1.0");
    host_run_frames(h, 1);
    test_state_guarded_minting(&a, "game:ticker@0.1.0", "ticker");
    a.cfg.persistent = 64;
    host_load(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}

void test_scan_shares(host_t *h)
{
    bridge_artifact_t a;

    h->cfg.budget.persistent = 192;
    h->cfg.budget.direct_indexed = 1;
    if (host_open(h))
        return;
    load_namer(h);
    test_load_guarded(h, "game:nested@0.1.0", "nested");
    test_load_guarded_minting(h, "game:blank@0.1.0", "namer-blank");
    test_load_guarded_minting(h, "game:aim@0.1.0", "aim");
    test_load_guarded(h, "game:hearer@0.1.0", "hearer");
    test_state_guarded_minting(&a, "game:ticker@0.1.0", "ticker");
    a.cfg.persistent = 64;
    host_load(h, &a);
    test_state_guarded(&a, "game:marker@0.1.0", "marker");
    a.cfg.persistent = 64;
    host_load(h, &a);
    host_run_frames(h, 1);
    host_unload(h, "game:ticker@0.1.0");
    test_state_guarded_minting(&a, "game:ticker@0.2.0", "ticker");
    a.cfg.persistent = 64;
    host_load(h, &a);
    host_unload(h, "game:namer@0.1.0");
    host_close(h);
}
