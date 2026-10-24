/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_inside.h"

int bridge_module_usage_now(bridge_world_t *w, bridge_module_usage_row_t *out,
    int max)
{
    const module_t *m = w->modules;
    int count = 0;

    while (m) {
        if (out && count < max) {
            out[count].identity = m->identity;
            out[count].persistent = m->arena_share;
            out[count].systems = m->system_count;
            out[count].observers = m->observer_count;
            out[count].entities = m->id_count;
            out[count].pad = 0;
        }
        count++;
        m = m->next;
    }
    return (count);
}

int bridge_module_usage(bridge_world_t *w, bridge_module_usage_row_t *out,
    int max)
{
    bool owner;
    int result;

    if (!bridge_enter(w, &owner))
        return (-1);
    result = bridge_module_usage_now(w, out, max);
    bridge_leave(w, owner);
    return (result);
}
