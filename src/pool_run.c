/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** src/pool_run.c -- one run of the pool: a batch of jobs shared between the
** workers and the caller.
*/

#include <string.h>

#include "pool.h"

/*
** run_alone
**
** Runs a batch on the calling thread, in order.
*/
static void run_alone(const pool_batch_t *batch)
{
    uint32_t i = 0;

    while (i < batch->count) {
        batch->fn(batch->jobs[i]);
        i++;
    }
}

/*
** begin_run
**
** Makes a batch the run in progress, with none of its jobs taken, and wakes
** as many workers as there are jobs beyond the caller's first.
*/
static void begin_run(pool_t *p, const pool_batch_t *batch)
{
    uint32_t i = 0;

    os_mutex_lock(p->lock);
    memset(p->taken, 0, batch->count);
    p->run = *batch;
    p->left = batch->count;
    os_mutex_unlock(p->lock);
    while (i < p->worker_count && i + 1 < batch->count) {
        os_event_set(p->workers[i].wake);
        i++;
    }
}

/*
** end_run
**
** Waits until every job of the run is done, and then leaves no job to take:
** nothing of a run may be taken once the caller's arrays are gone.
*/
static void end_run(pool_t *p)
{
    uint32_t left = 1;

    while (left) {
        os_event_wait(p->done, OS_FOREVER);
        os_mutex_lock(p->lock);
        left = p->left;
        os_mutex_unlock(p->lock);
    }
    os_mutex_lock(p->lock);
    p->run.count = 0;
    os_mutex_unlock(p->lock);
}

/*
** pool_run
**
** Runs every job of a batch and returns when all have. One job, or no worker
** to share with: no thread is woken for it.
*/
void pool_run(pool_t *p, const pool_batch_t *batch)
{
    if (batch->count < 2 || !p->worker_count) {
        run_alone(batch);
        return;
    }
    begin_run(p, batch);
    pool_run_taken(p, 1);
    end_run(p);
}
