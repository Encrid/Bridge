/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
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

int bridge_unload_now(bridge_t *b, const char *identity)
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
