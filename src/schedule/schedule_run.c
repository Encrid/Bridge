/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "schedule.h"

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
    block_note_work(w, job->m);
    for (uint32_t i = 0; i < job->count; i++) {
        if (!schedule_is_due(job->list[i]))
            continue;
        count_failure(w, job->list[i]);
        schedule_note_end(w, job->list[i]);
    }
}

void schedule_run_batch(schedule_pass_t *pass)
{
    bridge_world_t *w = pass->w;
    pool_batch_t batch = { schedule_run_job, w->run_handed, w->run_callers,
        pass->count };

    if (w->validate)
        pool_run_alone(&batch);
    else
        pool_run(w->pool, &batch);
    for (uint32_t k = 0; k < pass->count; k++)
        account_job(w, w->run_handed[k]);
    pass->count = 0;
}

static void run_jobs(schedule_pass_t *pass, uint32_t first)
{
    const bridge_world_t *w = pass->w;
    schedule_job_t *jobs = w->run_jobs;
    uint32_t k = first;
    uint32_t level;

    while (k < w->job_count && jobs[k].phase == pass->phase) {
        level = jobs[k].level;
        while (k < w->job_count && jobs[k].phase == pass->phase
            && jobs[k].level == level) {
            schedule_hand_job(pass, &jobs[k]);
            k++;
        }
        schedule_run_batch(pass);
    }
}

void schedule_run(bridge_world_t *w, uint32_t phase)
{
    schedule_pass_t pass = { .w = w, .phase = phase };

    if (phase == w->phase_count)
        schedule_run_triggered(&pass);
    else
        run_jobs(&pass, schedule_find_jobs(w, phase));
}
