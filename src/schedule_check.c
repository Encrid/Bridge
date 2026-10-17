/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "schedule.h"

static int check_cycle(schedule_phase_t *p)
{
    uint32_t i = schedule_get_cycle(&p->reach);

    if (i == p->n)
        return (0);
    snprintf(p->why, p->why_len, "'%.80s' would run before itself %s: the "
        "[order] pairs and the orders that the modules state make a cycle",
        p->in[i]->full, p->where);
    return (1);
}

static bool is_unordered(const schedule_phase_t *p, uint32_t i, uint32_t j)
{
    const bridge_system_t *s = p->in[i];
    const bridge_system_t *t = p->in[j];
    bool changing;
    bool ordered;

    if (s->m == t->m)
        ordered = schedule_is_stated_before(s, t)
            || schedule_is_stated_before(t, s);
    else
        ordered = schedule_orders(p->order, s, t)
            || schedule_orders(p->order, t, s);
    return (!ordered && schedule_find_conflict(s, t, &changing) != nullptr);
}

static void say_unordered(schedule_phase_t *p, uint32_t i, uint32_t j)
{
    bool changing;
    const char *shared = schedule_find_conflict(p->in[i], p->in[j], &changing);
    const char *verb = changing ? "attach or detach" : "touch";
    const char *tail = changing ? "" : ", one of them writing it";
    const char *none = "no pair orders them";

    if (p->in[i]->m == p->in[j]->m)
        none = "their module states no order of them";
    snprintf(p->why, p->why_len, "'%.80s' and '%.80s' both %s '%.60s' %s%s, "
        "and %s", p->in[i]->full, p->in[j]->full, verb, shared, p->where,
        tail, none);
}

static int check_system(schedule_phase_t *p, uint32_t i)
{
    uint32_t j = i + 1;

    while (j < p->n && !is_unordered(p, i, j))
        j++;
    if (j == p->n)
        return (0);
    say_unordered(p, i, j);
    return (1);
}

int schedule_check_phase(schedule_phase_t *p)
{
    uint32_t i = 0;

    if (check_cycle(p))
        return (1);
    while (i < p->n && !check_system(p, i))
        i++;
    return (i < p->n);
}
