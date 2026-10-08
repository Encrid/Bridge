/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/schedule_check.c -- what a schedule refuses: a system before itself,
** and two that conflict with nothing to order them.
*/

#include <stdio.h>

#include "schedule.h"

/*
** is_unordered
**
** Answers whether two systems of two modules conflict and neither runs
** before the other.
*/
static bool is_unordered(const schedule_phase_t *p, uint32_t i, uint32_t j)
{
    bool changing;

    if (p->in[i]->m == p->in[j]->m)
        return (false);
    if (schedule_reaches(&p->reach, i, j) || schedule_reaches(&p->reach, j, i))
        return (false);
    return (schedule_find_conflict(p->in[i], p->in[j], &changing) != nullptr);
}

/*
** say_unordered
**
** Writes why two systems cannot be scheduled: what they both touch, or both
** attach or detach, and where.
*/
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

/*
** check_system
**
** Refuses system `i` if it runs before itself, or if a later system
** conflicts with it and nothing orders the two.
*/
static int check_system(schedule_phase_t *p, uint32_t i)
{
    uint32_t j = i + 1;

    if (schedule_reaches(&p->reach, i, i)) {
        snprintf(p->why, p->why_len, "[order] has '%.100s' run before itself",
            p->in[i]->full);
        return (1);
    }
    while (j < p->n && !is_unordered(p, i, j))
        j++;
    if (j == p->n)
        return (0);
    say_unordered(p, i, j);
    return (1);
}

/*
** schedule_check_phase
**
** Refuses a phase that cannot be scheduled, at the first system that says
** why.
*/
int schedule_check_phase(schedule_phase_t *p)
{
    uint32_t i = 0;

    while (i < p->n && !check_system(p, i))
        i++;
    return (i < p->n);
}
