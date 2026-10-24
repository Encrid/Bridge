/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "schedule.h"

static uint32_t count_levels(const bridge_world_t *w, uint32_t phase)
{
    uint32_t levels = 0;
    uint32_t i = 0;

    while (i < w->system_count) {
        if (w->systems[i]->phase == phase && w->systems[i]->level >= levels)
            levels = w->systems[i]->level + 1;
        i++;
    }
    return (levels);
}

static void count_failure(bridge_world_t *w, bridge_system_t *s)
{
    say_t about = { s->m, s, BRIDGE_ERR };
    module_t *m = s->m;

    if (!s->failed && s->failures)
        s->failures--;
    if (!s->failed)
        return;
    s->failures++;
    if (m->max_failures > 0 && s->failures >= m->max_failures && !m->reached)
        m->reached = s;
    say_about(w, &about, "%s '%s' failed: %s",
        s->observer ? "observer" : "system", s->name, s->why);
}

static void account_job(bridge_world_t *w, const schedule_job_t *job)
{
    uint32_t i = 0;

    while (i < job->count) {
        count_failure(w, job->list[i]);
        schedule_note_end(w, job->list[i]);
        i++;
    }
}

static void run_level(schedule_pass_t *pass)
{
    bridge_world_t *w = pass->w;
    const schedule_job_t *jobs = w->run_jobs;
    pool_batch_t batch = { schedule_run_job, w->run_handed, w->run_callers };
    uint32_t k = 0;

    schedule_gather_jobs(pass);
    batch.count = pass->count;
    if (w->validate)
        pool_run_alone(&batch);
    else
        pool_run(w->pool, &batch);
    while (k < pass->count) {
        account_job(w, &jobs[k]);
        k++;
    }
}

void schedule_run(bridge_world_t *w, uint32_t phase)
{
    schedule_pass_t pass = { .w = w, .phase = phase };
    uint32_t levels = count_levels(w, phase);

    while (pass.level < levels) {
        run_level(&pass);
        pass.level++;
    }
}
