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

bool schedule_is_unordered(const schedule_pairs_t *order,
    const bridge_system_t *s, const bridge_system_t *t)
{
    bool changing;
    bool ordered;

    if (s->m == t->m)
        ordered = schedule_is_stated_before(s, t)
            || schedule_is_stated_before(t, s);
    else
        ordered = schedule_orders(order, s, t)
            || schedule_orders(order, t, s);
    return (!ordered && schedule_find_conflict(s, t, &changing) != nullptr);
}

static void say_unordered(schedule_phase_t *p, const bridge_system_t *a,
    const bridge_system_t *b)
{
    bool changing;
    const char *shared = schedule_find_conflict(a, b, &changing);
    const char *verb = changing ? "attach or detach" : "touch";
    const char *tail = changing ? "" : ", one of them writing it";
    const char *none = "no pair orders them";

    if (a->m == b->m)
        none = "their module states no order of them";
    snprintf(p->why, p->why_len, "'%.80s' and '%.80s' both %s '%.60s' %s%s, "
        "and %s", a->full, b->full, verb, shared, p->where, tail, none);
}

int schedule_check_phase(schedule_phase_t *p)
{
    const bridge_system_t *pair[2];

    if (check_cycle(p))
        return (1);
    if (!schedule_find_unordered(p, pair))
        return (0);
    say_unordered(p, pair[0], pair[1]);
    return (1);
}
