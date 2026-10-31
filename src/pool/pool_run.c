/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdatomic.h>

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
    uint32_t shared = 0;
    uint32_t i = 0;

    p->run = *batch;
    for (uint32_t j = 0; j < batch->count; j++) {
        if (!batch->caller_only[j]) {
            p->order[shared] = j;
            shared++;
        }
    }
    atomic_store_explicit(&p->left, batch->count, memory_order_relaxed);
    atomic_store_explicit(&p->claim, (uint64_t)shared << 32,
        memory_order_release);
    while (i < p->worker_count && i + 1 < batch->count) {
        os_event_set(p->workers[i].wake);
        i++;
    }
}

static void run_pinned(pool_t *p)
{
    for (uint32_t i = 0; i < p->run.count; i++) {
        if (p->run.caller_only[i]) {
            p->run.fn(p->run.jobs[i]);
            pool_finish_job(p);
        }
    }
}

void pool_run(pool_t *p, const pool_batch_t *batch)
{
    if (batch->count < 2 || !p->worker_count) {
        pool_run_alone(batch);
        return;
    }
    begin_run(p, batch);
    run_pinned(p);
    pool_run_taken(p);
    os_event_wait(p->done, OS_FOREVER);
}
