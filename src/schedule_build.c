/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
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
