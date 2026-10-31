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

void schedule_set_levels(schedule_phase_t *p)
{
    bool raised = true;

    for (uint32_t i = 0; i < p->n; i++)
        p->in[i]->level = 0;
    while (raised)
        raised = raise_all(p);
}
