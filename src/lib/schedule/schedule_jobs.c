/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "schedule.h"

static bool has_job(const schedule_pass_t *pass, const module_t *m)
{
    const schedule_job_t *jobs = pass->w->run_jobs;
    uint32_t k = 0;

    while (k < pass->count && jobs[k].m != m)
        k++;
    return (k < pass->count);
}

static bool runs_now(const schedule_pass_t *pass, const bridge_system_t *s)
{
    if (s->phase != pass->phase || s->level != pass->level)
        return (false);
    if (s->m->stopped)
        return (false);
    return (!s->observer || s->pending_count);
}

static void list_system(schedule_pass_t *pass, schedule_job_t *job,
    bridge_system_t *s)
{
    const bridge_world_t *w = pass->w;
    uint64_t kind = pass->phase == w->phase_count ? 0 : TURN_SYSTEM;

    s->turn = kind | pass->ordinal;
    s->place = pass->ordinal;
    s->failed = 0;
    s->ended = 0;
    job->list[job->count] = s;
    job->count++;
    pass->ordinal++;
}

static void fill_job(schedule_pass_t *pass, uint32_t i)
{
    bridge_world_t *w = pass->w;
    schedule_job_t *job = (schedule_job_t *)w->run_jobs + pass->count;
    uint32_t k = i;

    job->m = w->systems[i]->m;
    job->list = (bridge_system_t **)w->run_lists + pass->listed;
    job->count = 0;
    while (k < w->system_count) {
        if (w->systems[k]->m == job->m && runs_now(pass, w->systems[k]))
            list_system(pass, job, w->systems[k]);
        k++;
    }
    if (!job->count)
        return;
    pass->listed += job->count;
    w->run_handed[pass->count] = job;
    w->run_callers[pass->count] = (uint8_t)job->m->pinned;
    pass->count++;
}

void schedule_gather_jobs(schedule_pass_t *pass)
{
    const bridge_world_t *w = pass->w;
    uint32_t i = 0;

    pass->count = 0;
    pass->listed = 0;
    while (i < w->system_count) {
        if (!has_job(pass, w->systems[i]->m))
            fill_job(pass, i);
        i++;
    }
}
