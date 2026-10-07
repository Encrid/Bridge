/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** src/schedule_jobs.c -- the jobs of one level of a phase: a module's
** systems that run in it, gathered into one list.
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

/*
** runs_now
**
** Answers whether a system runs in this level of this phase: it is of them,
** its module runs, it is not parked, and if it is an observer something
** triggered it.
*/
static bool runs_now(const schedule_pass_t *pass, const bridge_system_t *s)
{
    if (s->phase != pass->phase || s->level != pass->level)
        return (false);
    if (s->m->stopped || s->parked)
        return (false);
    return (!s->observer || s->pending_count);
}

/*
** list_system
**
** Adds a system to a job and numbers it: its turn and when it runs, by the
** barriers so far and its place in this phase. An observer ran at the last
** barrier, before anything since, and its turn says so.
**
** threads.md, *Threads*
*/
static void list_system(schedule_pass_t *pass, schedule_job_t *job,
    bridge_system_t *s)
{
    const bridge_t *b = pass->b;
    uint32_t ordinal = pass->ordinal & 0xffffu;
    uint32_t kind = pass->phase == b->phase_count ? 0 : TURN_SYSTEM;

    s->turn = kind | ordinal;
    s->when = b->barriers << 16 | ordinal;
    s->failed = 0;
    job->list[job->count] = s;
    job->count++;
    pass->ordinal++;
}

/*
** fill_job
**
** Makes the next job of the level the one of the module of system `i`, with
** every system of that module from `i` on that runs now. An empty job is not
** counted.
*/
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

/*
** schedule_gather_jobs
**
** Makes the jobs of one level of a phase, one for each module that has a
** system running in it.
*/
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
