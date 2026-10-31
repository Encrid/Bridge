/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "schedule.h"

static void number_systems(bridge_world_t *w)
{
    for (uint32_t i = 0; i < w->system_count; i++)
        w->systems[i]->at = i;
}

static void make_group(bridge_world_t *w, schedule_phase_t *p)
{
    schedule_gather_group(w, p);
    schedule_add_stated(w, p);
    schedule_add_own(p);
}

static int check_phases(bridge_world_t *w, schedule_phase_t *p)
{
    int refused = 0;

    for (p->phase = 0; p->phase <= w->phase_count && !refused; p->phase++) {
        snprintf(p->where, sizeof(p->where), "at the barrier");
        if (p->phase < w->phase_count)
            snprintf(p->where, sizeof(p->where), "in phase '%.40s'",
                w->phases[p->phase]);
        make_group(w, p);
        refused = schedule_check_phase(p);
    }
    return (refused);
}

static void set_phase_levels(bridge_world_t *w, schedule_phase_t *p)
{
    for (p->phase = 0; p->phase <= w->phase_count; p->phase++) {
        make_group(w, p);
        schedule_set_levels(p);
    }
}

int schedule_build(bridge_world_t *w, char *why, size_t why_len)
{
    schedule_phase_t p = { .order = &w->order, .why = why,
        .why_len = why_len };

    number_systems(w);
    p.in = (bridge_system_t **)w->build_in;
    if (schedule_cross_phase(w, why, why_len) || check_phases(w, &p))
        return (1);
    set_phase_levels(w, &p);
    for (uint32_t i = 0; i < w->system_count; i++) {
        w->systems[i]->fresh = false;
        w->systems[i]->dirty = false;
    }
    schedule_commit_jobs(w);
    return (0);
}
