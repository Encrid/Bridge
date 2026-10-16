/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "schedule.h"

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
