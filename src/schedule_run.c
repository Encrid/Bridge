/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/schedule_run.c -- a phase run: a level at a time, one job per module,
** and what failed counted.
*/

#include "schedule.h"

/*
** count_levels
**
** Answers how many levels a phase has: one more than its highest.
*/
static uint32_t count_levels(const bridge_t *b, uint32_t phase)
{
    uint32_t levels = 0;
    uint32_t i = 0;

    while (i < b->system_count) {
        if (b->systems[i]->phase == phase && b->systems[i]->level >= levels)
            levels = b->systems[i]->level + 1;
        i++;
    }
    return (levels);
}

/*
** count_failure
**
** Counts what one system's run came to. A system is counted alone, so that
** one that always fails is not forgiven by another of its module's that
** never does. A failed invocation is abandoned, and logged.
**
** failure.md, *The budget*
*/
static void count_failure(bridge_t *b, bridge_system_t *s)
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
    say_about(b, &about, "%s '%s' failed: %s",
        s->observer ? "observer" : "system", s->name, s->why);
}

static void account_job(bridge_t *b, const schedule_job_t *job)
{
    uint32_t i = 0;

    while (i < job->count) {
        count_failure(b, job->list[i]);
        i++;
    }
}

/*
** run_level
**
** Runs one level of a phase: its jobs at once, and then what failed counted
** and said, here, so that the sink is only ever called on the thread that
** asked for the phase.
*/
static void run_level(schedule_pass_t *pass)
{
    bridge_t *b = pass->b;
    const schedule_job_t *jobs = b->run_jobs;
    pool_batch_t batch = { schedule_run_job, b->run_handed, b->run_callers };
    uint32_t k = 0;

    schedule_gather_jobs(pass);
    batch.count = pass->count;
    pool_run(b->pool, &batch);
    while (k < pass->count) {
        account_job(b, &jobs[k]);
        k++;
    }
}

/*
** schedule_run
**
** Runs the systems of one phase, a level at a time.
*/
void schedule_run(bridge_t *b, uint32_t phase)
{
    schedule_pass_t pass = { .b = b, .phase = phase };
    uint32_t levels = count_levels(b, phase);

    while (pass.level < levels) {
        run_level(&pass);
        pass.level++;
    }
}
