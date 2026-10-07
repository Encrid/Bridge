/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/reload_fail.c -- a reload that failed.
**
** failure.md, *When something fails*
*/

#include "reload.h"

/*
** undo_new
**
** Takes the new instance back out. What it would have moved stays as it is,
** under the type it has; the entities are the old record's still; and what
** the reload added to the share goes back.
*/
static void undo_new(reloading_t *r)
{
    bridge_t *b = r->b;
    module_t *n = r->n;
    declared_type_t *was;
    uint32_t i = 0;

    module_drop_systems(b, n);
    while (i < n->declared_count) {
        was = n->declared[i]->replaces;
        if (was)
            was->superseded = false;
        i++;
    }
    module_release_types(b, n, false);
    n->id_count = 0;
    n->id_next = 0;
    reload_give_back(r);
    module_discard(b, n);
}

/*
** keep_old
**
** The old instance is kept, and its systems are put back as they were. The
** new one's were dropped, so the places are there.
*/
static void keep_old(bridge_t *b)
{
    uint32_t i = 0;

    while (i < b->system_count) {
        abi_unpark(b, b->systems[i]);
        i++;
    }
}

/*
** stop_old
**
** There is no instance left to keep. The module stays, stopped, with
** everything it declared: a reload is a call that means "keep running", and
** those never discard. An artifact that works brings it back; an unload is
** what ends it.
**
** lifetime.md, *Lifetime*
*/
static void stop_old(bridge_t *b, module_t *old)
{
    say_t about = { old, nullptr, BRIDGE_ERR };

    module_drop_systems(b, old);
    old->stopped = 1;
    say_about(b, &about, "module '%s' is stopped: its reload failed after "
        "its old instance had been shut down. What it declared is kept",
        old->identity);
    if (old->on_error == BRIDGE_FAIL)
        bridge_stop(b, old->identity, "could not be reloaded");
}

/*
** reload_fail
**
** Says that a reload failed and takes the new instance back out. A schedule
** with its systems taken back out is one that was built before, so the
** rebuild cannot be refused.
**
** failure.md, *The budget*
*/
int reload_fail(reloading_t *r)
{
    bridge_t *b = r->b;
    const char *id = r->a->cfg.identity;

    say_error(b, "module '%s' was not reloaded: %s", id, r->err.text);
    undo_new(r);
    if (r->side_by_side)
        keep_old(b);
    else
        stop_old(b, r->old);
    (void)schedule_build(b, r->err.text, sizeof(r->err.text));
    return (1);
}
