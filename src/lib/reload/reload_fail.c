/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "reload.h"

static void undo_new(reloading_t *r)
{
    bridge_world_t *w = r->w;
    module_t *n = r->n;
    declared_type_t *was;
    uint32_t i = 0;

    module_drop_systems(w, n);
    while (i < n->declared_count) {
        was = n->declared[i]->replaces;
        if (was)
            was->superseded = false;
        i++;
    }
    module_release_types(w, n, false);
    n->id_count = 0;
    n->id_next = 0;
    reload_give_back(r);
    module_discard(w, n);
}

static void keep_old(bridge_world_t *w)
{
    uint32_t i = 0;

    while (i < w->system_count) {
        abi_unpark(w, w->systems[i]);
        i++;
    }
}

static void stop_old(bridge_world_t *w, module_t *old)
{
    say_t about = { old, nullptr, BRIDGE_ERR };

    module_drop_systems(w, old);
    old->stopped = 1;
    say_about(w, &about, "module '%s' is stopped: its reload failed after "
        "its old instance had been shut down. What it declared is kept",
        old->identity);
    if (old->on_error == BRIDGE_FAIL)
        bridge_stop(w, old->identity, "could not be reloaded");
}

int reload_fail(reloading_t *r)
{
    bridge_world_t *w = r->w;
    const char *id = r->a->cfg.identity;

    if (r->n->priv)
        block_drain(w, r->n);
    say_error(w, "module '%s' was not reloaded: %s", id, r->err.text);
    undo_new(r);
    if (r->side_by_side)
        keep_old(w);
    else
        stop_old(w, r->old);
    (void)schedule_build(w, r->err.text, sizeof(r->err.text));
    return (1);
}
