/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "module.h"

static module_t *find_loaded(bridge_world_t *w, const char *identity)
{
    module_t *m = w->modules;

    while (m && (!identity || strcmp(m->identity, identity)))
        m = m->next;
    return (m);
}

static void take_out(bridge_world_t *w, module_t *m)
{
    module_t **link = &w->modules;
    const uint64_t *ready = m->ids + m->id_next;

    module_drop_systems(w, m);
    module_release_types(w, m, true);
    storage_release(w->storage, ready, m->id_count - m->id_next);
    module_settle_share(w, m);
    while (*link != m)
        link = &(*link)->next;
    *link = m->next;
    w->module_count--;
    module_discard(w, m);
}

int bridge_unload_now(bridge_world_t *w, const char *identity)
{
    char why[256];
    module_t *m;

    if (bridge_check_call(w, identity))
        return (1);
    m = find_loaded(w, identity);
    if (!m) {
        say_error(w, "no module '%s' is loaded, so none is unloaded",
            identity ? identity : "");
        return (1);
    }
    bridge_give_up(w, m);
    module_drop_systems(w, m);
    module_shut_down(w, m);
    take_out(w, m);
    (void)schedule_build(w, why, sizeof(why));
    w->layout_moved = 1;
    block_barrier(w);
    return (0);
}
