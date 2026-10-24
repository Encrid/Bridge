/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "pool.h"

void pool_run_alone(const pool_batch_t *batch)
{
    uint32_t i = 0;

    while (i < batch->count) {
        batch->fn(batch->jobs[i]);
        i++;
    }
}

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

void pool_run(pool_t *p, const pool_batch_t *batch)
{
    if (batch->count < 2 || !p->worker_count) {
        pool_run_alone(batch);
        return;
    }
    begin_run(p, batch);
    pool_run_taken(p, 1);
    end_run(p);
}
