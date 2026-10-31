/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "schedule.h"

static void list_due(schedule_pass_t *pass, const schedule_job_t *job)
{
    uint64_t kind = pass->phase == pass->w->phase_count ? 0 : TURN_SYSTEM;
    bridge_system_t *s;

    for (uint32_t i = 0; i < job->count; i++) {
        s = job->list[i];
        if (!schedule_is_due(s))
            continue;
        s->turn = kind | pass->ordinal;
        s->place = pass->ordinal;
        s->failed = 0;
        s->ended = 0;
        pass->ordinal++;
    }
}

uint32_t schedule_find_jobs(const bridge_world_t *w, uint32_t phase)
{
    const schedule_job_t *jobs = w->run_jobs;
    uint32_t low = 0;
    uint32_t high = w->job_count;
    uint32_t mid;

    while (low < high) {
        mid = low + (high - low) / 2;
        if (jobs[mid].phase < phase)
            low = mid + 1;
        else
            high = mid;
    }
    return (low);
}

void schedule_hand_job(schedule_pass_t *pass, schedule_job_t *job)
{
    bridge_world_t *w = pass->w;

    if (job->m->stopped)
        return;
    list_due(pass, job);
    w->run_handed[pass->count] = job;
    w->run_callers[pass->count] = (uint8_t)job->m->pinned;
    pass->count++;
}
