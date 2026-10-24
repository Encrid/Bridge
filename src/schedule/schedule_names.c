/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "schedule.h"

uint32_t schedule_find_name(const schedule_pairs_t *order, const char *full)
{
    uint32_t at = 0;

    while (at < order->n && strcmp(order->names[at], full))
        at++;
    return (at);
}

bool schedule_orders(const schedule_pairs_t *order, const bridge_system_t *s,
    const bridge_system_t *t)
{
    if (s->named >= order->n || t->named >= order->n)
        return (false);
    return (schedule_reaches(&order->reach, s->named, t->named));
}

void schedule_resolve_names(bridge_world_t *w)
{
    for (uint32_t i = 0; i < w->system_count; i++)
        if (w->systems[i]->full)
            w->systems[i]->named = schedule_find_name(&w->order,
                w->systems[i]->full);
}

void schedule_free_pairs(schedule_pairs_t *o)
{
    for (uint32_t i = 0; i < o->n; i++)
        free(o->names[i]);
    free(o->names);
    free(o->pairs);
    free(o->reach.bits);
}
