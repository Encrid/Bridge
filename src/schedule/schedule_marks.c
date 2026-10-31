/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "schedule.h"

bool schedule_is_neighbor(const bridge_world_t *w, const bridge_system_t *s,
    const bridge_system_t *t)
{
    if (t == s || t->phase != s->phase || t->parked)
        return (false);
    if (s->m == t->m && (schedule_is_stated_before(s, t)
        || schedule_is_stated_before(t, s)))
        return (true);
    return (schedule_orders(&w->order, s, t)
        || schedule_orders(&w->order, t, s));
}

void schedule_mark_neighbors(bridge_world_t *w, const bridge_system_t *s)
{
    for (uint32_t i = 0; i < w->system_count; i++)
        if (schedule_is_neighbor(w, s, w->systems[i]))
            w->systems[i]->dirty = true;
}

void schedule_leave_module(bridge_world_t *w, const module_t *m)
{
    for (uint32_t i = 0; i < w->system_count; i++) {
        if (w->systems[i]->m == m) {
            schedule_mark_neighbors(w, w->systems[i]);
            schedule_unindex_system(w->systems[i]);
        }
    }
}

void schedule_mark_all(bridge_world_t *w)
{
    schedule_resolve_names(w);
    for (uint32_t i = 0; i < w->system_count; i++) {
        w->systems[i]->fresh = true;
        w->systems[i]->dirty = true;
    }
}
