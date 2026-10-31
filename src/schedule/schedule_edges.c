/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "schedule.h"

static void add_stated_from(const bridge_world_t *w, schedule_phase_t *p,
    uint32_t i)
{
    for (uint32_t j = 0; j < p->n; j++)
        if (schedule_orders(&w->order, p->in[i], p->in[j]))
            schedule_add_edge(&p->reach, i, j);
}

void schedule_add_stated(const bridge_world_t *w, schedule_phase_t *p)
{
    for (uint32_t i = 0; i < p->n; i++)
        add_stated_from(w, p, i);
}

static void order_after(schedule_phase_t *p, uint32_t i)
{
    const bridge_system_t *s = p->in[i];

    for (uint32_t j = 0; j < p->n; j++)
        if (s->m == p->in[j]->m && schedule_is_stated_before(s, p->in[j]))
            schedule_add_edge(&p->reach, i, j);
    schedule_join_through(&p->reach, i);
}

void schedule_add_own(schedule_phase_t *p)
{
    uint32_t i = 0;

    while (i < p->n) {
        order_after(p, i);
        i++;
    }
}
