/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "schedule.h"

static const bridge_system_t *system_of(const bridge_world_t *w,
    uint32_t named)
{
    const bridge_system_t *found = nullptr;

    for (uint32_t i = 0; i < w->system_count; i++)
        if (w->systems[i]->named == named)
            found = w->systems[i];
    return (found);
}

int schedule_cross_phase(const bridge_world_t *w, char *why, size_t why_len)
{
    const schedule_pairs_t *order = &w->order;
    const bridge_system_t *a;
    const bridge_system_t *b;

    for (uint32_t k = 0; k < order->pair_count; k++) {
        a = system_of(w, order->pairs[2 * k]);
        b = system_of(w, order->pairs[2 * k + 1]);
        if (a && b && a->phase != b->phase) {
            snprintf(why, why_len, "the [order] pair of '%.80s' and '%.80s' "
                "orders nothing: they run in different phases", a->full,
                b->full);
            return (1);
        }
    }
    return (0);
}
