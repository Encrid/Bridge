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

static bool is_crossing(const bridge_world_t *w, uint32_t k)
{
    const bridge_system_t *a = system_of(w, w->order.pairs[2 * k]);
    const bridge_system_t *b = system_of(w, w->order.pairs[2 * k + 1]);

    return (a && b && a->phase != b->phase);
}

static uint32_t get_first_crossing(const bridge_world_t *w,
    const bridge_system_t *s, uint32_t first)
{
    const uint32_t *pairs = w->order.pairs;

    for (uint32_t k = 0; k < first; k++)
        if ((pairs[2 * k] == s->named || pairs[2 * k + 1] == s->named)
            && is_crossing(w, k))
            return (k);
    return (first);
}

int schedule_cross_phase(const bridge_world_t *w, char *why, size_t why_len)
{
    const schedule_pairs_t *order = &w->order;
    uint32_t first = order->pair_count;

    for (uint32_t i = 0; i < w->system_count; i++)
        if (w->systems[i]->fresh)
            first = get_first_crossing(w, w->systems[i], first);
    if (first == order->pair_count)
        return (0);
    snprintf(why, why_len, "the [order] pair of '%.80s' and '%.80s' orders "
        "nothing: they run in different phases",
        system_of(w, order->pairs[2 * first])->full,
        system_of(w, order->pairs[2 * first + 1])->full);
    return (1);
}
