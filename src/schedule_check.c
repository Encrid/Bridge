/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "schedule.h"

static bool is_unordered(const schedule_phase_t *p, uint32_t i, uint32_t j)
{
    bool changing;

    if (p->in[i]->m == p->in[j]->m)
        return (false);
    if (schedule_orders(p->order, p->in[i], p->in[j])
        || schedule_orders(p->order, p->in[j], p->in[i]))
        return (false);
    return (schedule_find_conflict(p->in[i], p->in[j], &changing) != nullptr);
}

static void say_unordered(schedule_phase_t *p, uint32_t i, uint32_t j)
{
    bool changing;
    const char *shared = schedule_find_conflict(p->in[i], p->in[j], &changing);
    const char *verb = changing ? "attach or detach" : "touch";
    const char *tail = changing ? "" : ", one of them writing it";
    char place[64] = "at the barrier";

    if (p->where)
        snprintf(place, sizeof(place), "in phase '%.40s'", p->where);
    snprintf(p->why, p->why_len, "'%.80s' and '%.80s' both %s '%.60s' %s%s, "
        "and nothing orders them", p->in[i]->full, p->in[j]->full, verb,
        shared, place, tail);
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

    while (i < p->n && !check_system(p, i))
        i++;
    return (i < p->n);
}
