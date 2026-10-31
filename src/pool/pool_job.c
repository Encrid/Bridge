/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdatomic.h>

#include "pool.h"

static int take_job(pool_t *p)
{
    uint64_t claim = atomic_fetch_add_explicit(&p->claim, 1,
        memory_order_acquire);

    if ((uint32_t)claim >= (uint32_t)(claim >> 32))
        return (-1);
    return ((int)p->order[(uint32_t)claim]);
}

void pool_finish_job(pool_t *p)
{
    uint32_t left = atomic_fetch_sub_explicit(&p->left, 1,
        memory_order_acq_rel);

    if (left == 1)
        os_event_set(p->done);
}

void pool_run_taken(pool_t *p)
{
    int job = take_job(p);

    while (job >= 0) {
        p->run.fn(p->run.jobs[job]);
        pool_finish_job(p);
        job = take_job(p);
    }
}

void pool_work(void *arg)
{
    pool_worker_t *w = arg;

    os_event_wait(w->wake, OS_FOREVER);
    while (!w->p->quit) {
        pool_run_taken(w->p);
        os_event_wait(w->wake, OS_FOREVER);
    }
}
