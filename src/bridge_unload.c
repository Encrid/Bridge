/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/bridge_unload.c -- a module unloaded.
**
** lifetime.md, *What unload leaves behind*
*/

#include <string.h>

#include "module.h"

static module_t *find_loaded(bridge_t *b, const char *identity)
{
    module_t *m = b->modules;

    while (m && (!identity || strcmp(m->identity, identity)))
        m = m->next;
    return (m);
}

/*
** take_out
**
** The rest of an unload, once the module's code has run for the last time:
** what it declared goes by its policy, the ids held ready for it are
** entities nobody will ever spawn, and its share of the arena goes back to
** it unless a row that is staying holds a string there.
*/
static void take_out(bridge_t *b, module_t *m)
{
    module_t **link = &b->modules;
    const uint64_t *ready = m->ids + m->id_next;

    module_drop_systems(b, m);
    module_release_types(b, m, true);
    storage_release(b->storage, ready, m->id_count - m->id_next);
    module_settle_share(b, m);
    while (*link != m)
        link = &(*link)->next;
    *link = m->next;
    b->module_count--;
    module_discard(b, m);
}

/*
** bridge_unload
**
** Unloads a module. A stopped deck takes an unload of the module that
** stopped it, and of no other. Its systems are out of the schedule before
** its last call; and after the barrier what left is out of every module's
** spans, and observed.
**
** lifecycle.md, *The runtime lifecycle*
*/
int bridge_unload(bridge_t *b, const char *identity)
{
    char why[256];
    module_t *m;

    if (bridge_check_call(b, identity))
        return (1);
    m = find_loaded(b, identity);
    if (!m) {
        say_error(b, "no module '%s' is loaded, so none is unloaded",
            identity ? identity : "");
        return (1);
    }
    bridge_give_up(b, m);
    module_drop_systems(b, m);
    module_shut_down(b, m);
    take_out(b, m);
    (void)schedule_build(b, why, sizeof(why));
    b->layout_moved = 1;
    block_barrier(b);
    return (0);
}
