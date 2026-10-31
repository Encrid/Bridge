/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "schedule.h"

static bool raise_after(schedule_phase_t *p, uint32_t i)
{
    uint32_t level = p->in[i]->level;
    bool raised = false;

    for (uint32_t j = 0; j < p->n; j++) {
        if (i != j && schedule_reaches(&p->reach, i, j)
            && p->in[j]->level <= level) {
            p->in[j]->level = level + 1;
            raised = true;
        }
    }
    return (raised);
}

static bool raise_all(schedule_phase_t *p)
{
    bool raised = false;

    for (uint32_t i = 0; i < p->n; i++)
        raised = raise_after(p, i) || raised;
    return (raised);
}

static void set_levels(schedule_phase_t *p, bool refused)
{
    bool raised = !refused;

    for (uint32_t i = 0; i < p->n; i++)
        p->in[i]->level = 0;
    while (raised)
        raised = raise_all(p);
}

static int build_phase(bridge_world_t *w, schedule_phase_t *p)
{
    int refused;

    snprintf(p->where, sizeof(p->where), "at the barrier");
    if (p->phase < w->phase_count)
        snprintf(p->where, sizeof(p->where), "in phase '%.40s'",
            w->phases[p->phase]);
    schedule_gather_phase(w, p);
    schedule_add_stated(w, p);
    schedule_add_own(p);
    refused = schedule_check_phase(p);
    set_levels(p, refused);
    return (refused);
}

int schedule_build(bridge_world_t *w, char *why, size_t why_len)
{
    schedule_phase_t p = { .order = &w->order, .why = why,
        .why_len = why_len };
    int refused = 0;

    schedule_resolve_names(w);
    if (schedule_cross_phase(w, why, why_len))
        return (1);
    p.in = (bridge_system_t **)w->build_in;
    while (p.phase <= w->phase_count && !refused) {
        refused = build_phase(w, &p);
        p.phase++;
    }
    if (!refused)
        schedule_commit_jobs(w);
    return (refused);
}
