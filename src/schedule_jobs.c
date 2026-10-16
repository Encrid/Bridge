/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "schedule.h"

static bool has_job(const schedule_pass_t *pass, const module_t *m)
{
    const schedule_job_t *jobs = pass->b->run_jobs;
    uint32_t k = 0;

    while (k < pass->count && jobs[k].m != m)
        k++;
    return (k < pass->count);
}

static bool runs_now(const schedule_pass_t *pass, const bridge_system_t *s)
{
    if (s->phase != pass->phase || s->level != pass->level)
        return (false);
    if (s->m->stopped || s->parked)
        return (false);
    return (!s->observer || s->pending_count);
}

static void list_system(schedule_pass_t *pass, schedule_job_t *job,
    bridge_system_t *s)
{
    const bridge_t *b = pass->b;
    uint64_t kind = pass->phase == b->phase_count ? 0 : TURN_SYSTEM;

    s->turn = kind | pass->ordinal;
    s->place = pass->ordinal;
    s->failed = 0;
    job->list[job->count] = s;
    job->count++;
    pass->ordinal++;
}

static void fill_job(schedule_pass_t *pass, uint32_t i)
{
    bridge_t *b = pass->b;
    schedule_job_t *job = (schedule_job_t *)b->run_jobs + pass->count;
    uint32_t k = i;

    job->m = b->systems[i]->m;
    job->list = (bridge_system_t **)b->run_lists + pass->listed;
    job->count = 0;
    while (k < b->system_count) {
        if (b->systems[k]->m == job->m && runs_now(pass, b->systems[k]))
            list_system(pass, job, b->systems[k]);
        k++;
    }
    if (!job->count)
        return;
    pass->listed += job->count;
    b->run_handed[pass->count] = job;
    b->run_callers[pass->count] = (uint8_t)job->m->pinned;
    pass->count++;
}

void schedule_gather_jobs(schedule_pass_t *pass)
{
    const bridge_t *b = pass->b;
    uint32_t i = 0;

    pass->count = 0;
    pass->listed = 0;
    while (i < b->system_count) {
        if (!has_job(pass, b->systems[i]->m))
            fill_job(pass, i);
        i++;
    }
}
