/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "schedule.h"

void schedule_gather_phase(bridge_world_t *w, schedule_phase_t *p)
{
    uint32_t i = 0;
    size_t words;

    p->n = 0;
    while (i < w->system_count) {
        if (w->systems[i]->phase == p->phase && !w->systems[i]->parked) {
            p->in[p->n] = w->systems[i];
            p->n++;
        }
        i++;
    }
    p->reach.n = p->n;
    p->reach.words = schedule_get_words(p->n);
    p->reach.bits = w->build_bits;
    words = schedule_get_matrix_words(p->n);
    memset(p->reach.bits, 0, words * sizeof(*p->reach.bits));
}

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
