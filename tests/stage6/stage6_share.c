/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/stage6/stage6_share.c -- a module with sixteen bytes of the arena,
** reloaded into one whose reload takes forty. Stating more gives it more;
** the same leaves it short; less is refused. With another module's share
** carved after its own, the more is a second range.
**
** budgets.md, *Budgets*
*/

#include <string.h>

#include "stage6.h"

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

    stage6_state_wasm(&a, "game:namer@0.1.0", "namer");
    a.cfg.persistent = 64;
    host_load(h, &a);
}

/*
** stage6_grow_share
**
** Loads the ticker with sixteen bytes and reloads it stating what the case
** is named after.
*/
void stage6_grow_share(host_t *h)
{
    bridge_artifact_t a;

    if (host_open(h))
        return;
    stage6_state_ticker(&a, "game:ticker@0.1.0", "ticker");
    a.cfg.persistent = 16;
    host_load(h, &a);
    if (!strcmp(h->what, "sharechain"))
        load_namer(h);
    host_run_frames(h, 2);
    stage6_state_ticker(&a, "game:ticker@0.2.0", "ticker-alloc");
    a.cfg.persistent = get_next_share(h->what);
    host_reload(h, &a);
    host_run_frames(h, 2);
    stage6_say_modules(h);
    host_close(h);
}

/*
** stage6_use_arena
**
** The whole arena is one module's share, and it keeps a string there.
** Alone, its unload gives the share back and the next module has it. With
** another module that declares the same component the row stays, the
** string with it, and the share is not given to anybody: the reader still
** says it, after the refusal as before.
**
** strings.md, *It is given back when nothing points into it*
*/
void stage6_use_arena(host_t *h)
{
    bool kept = !strcmp(h->what, "arenakept");
    bridge_artifact_t a;

    h->cfg.budget.persistent = 64;
    if (host_open(h))
        return;
    load_namer(h);
    if (kept)
        stage6_load_wasm(h, "game:reader@0.1.0", "namer-reader");
    host_run_frames(h, 1);
    host_unload(h, "game:namer@0.1.0");
    host_run_frames(h, 1);
    stage6_state_wasm(&a, "game:ticker@0.1.0", "ticker");
    a.cfg.persistent = 64;
    host_load(h, &a);
    host_run_frames(h, 1);
    host_close(h);
}
