/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/schedule_build.c -- a schedule built: for every phase, edges, the
** refusal, then levels.
*/

#include "schedule.h"

/*
** raise_after
**
** Puts every system that `i` runs before at a level after its own.
*/
static void raise_after(schedule_phase_t *p, uint32_t i)
{
    uint32_t level = p->in[i]->level;
    uint32_t j = 0;

    while (j < p->n) {
        if (i != j && schedule_reaches(&p->reach, i, j)
            && p->in[j]->level <= level)
            p->in[j]->level = level + 1;
        j++;
    }
}

static void raise_all(schedule_phase_t *p)
{
    uint32_t i = 0;

    while (i < p->n) {
        raise_after(p, i);
        i++;
    }
}

/*
** set_levels
**
** Gives each system its level: a level is everything whose predecessors are
** all in earlier ones. A refused phase is left with every system at level 0.
*/
static void set_levels(schedule_phase_t *p, bool refused)
{
    uint32_t i = 0;
    uint32_t k = 0;

    while (i < p->n) {
        p->in[i]->level = 0;
        i++;
    }
    while (k < p->n && !refused) {
        raise_all(p);
        k++;
    }
}

/*
** build_phase
**
** Schedules one phase: what the deployment orders, then what each module
** orders of its own, each closed over, then the refusal and the levels.
*/
static int build_phase(bridge_t *b, schedule_phase_t *p)
{
    int refused;

    schedule_gather_phase(b, p);
    schedule_add_stated(b, p);
    schedule_close_matrix(&p->reach);
    schedule_add_own(p);
    schedule_close_matrix(&p->reach);
    refused = schedule_check_phase(p);
    set_levels(p, refused);
    return (refused);
}

/*
** schedule_build
**
** Schedules every phase, and the barrier after them as one more.
*/
int schedule_build(bridge_t *b, char *why, size_t why_len)
{
    schedule_phase_t p = { .why = why, .why_len = why_len };
    int refused = 0;

    p.in = (bridge_system_t **)b->build_in;
    while (p.phase <= b->phase_count && !refused) {
        p.where = p.phase < b->phase_count ? b->phases[p.phase] : nullptr;
        refused = build_phase(b, &p);
        p.phase++;
    }
    return (refused);
}
