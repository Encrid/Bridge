/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
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
