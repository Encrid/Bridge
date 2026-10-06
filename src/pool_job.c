/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/pool_job.c -- a job of a run: taken by one thread, run, and counted
** as done.
*/

#include "pool.h"

/*
** take_job
**
** Answers the next job nobody took and marks it taken, or -1. A job marked
** for the caller is the caller's alone.
*/
static int take_job(pool_t *p, int caller)
{
    int found = -1;
    uint32_t i = 0;

    os_mutex_lock(p->lock);
    while (i < p->run.count && found < 0) {
        if (!p->taken[i] && (caller || !p->run.caller_only[i])) {
            p->taken[i] = 1;
            found = (int)i;
        }
        i++;
    }
    os_mutex_unlock(p->lock);
    return (found);
}

/*
** finish_job
**
** Counts one job as done, and sets the pool's event when it was the last.
*/
static void finish_job(pool_t *p)
{
    os_mutex_lock(p->lock);
    p->left--;
    if (!p->left)
        os_event_set(p->done);
    os_mutex_unlock(p->lock);
}

/*
** pool_run_taken
**
** Takes jobs of the run in progress and runs them, until none is left to
** take.
*/
void pool_run_taken(pool_t *p, int caller)
{
    int job = take_job(p, caller);

    while (job >= 0) {
        p->run.fn(p->run.jobs[job]);
        finish_job(p);
        job = take_job(p, caller);
    }
}

/*
** pool_work
**
** What a worker's thread runs: it sleeps until woken, runs what it can take,
** and ends when the pool is told to quit.
*/
void pool_work(void *arg)
{
    pool_worker_t *w = arg;

    os_event_wait(w->wake, OS_FOREVER);
    while (!w->p->quit) {
        pool_run_taken(w->p, 0);
        os_event_wait(w->wake, OS_FOREVER);
    }
}
