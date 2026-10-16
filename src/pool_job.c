/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "pool.h"

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

static void finish_job(pool_t *p)
{
    os_mutex_lock(p->lock);
    p->left--;
    if (!p->left)
        os_event_set(p->done);
    os_mutex_unlock(p->lock);
}

void pool_run_taken(pool_t *p, int caller)
{
    int job = take_job(p, caller);

    while (job >= 0) {
        p->run.fn(p->run.jobs[job]);
        finish_job(p);
        job = take_job(p, caller);
    }
}

void pool_work(void *arg)
{
    pool_worker_t *w = arg;

    os_event_wait(w->wake, OS_FOREVER);
    while (!w->p->quit) {
        pool_run_taken(w->p, 0);
        os_event_wait(w->wake, OS_FOREVER);
    }
}
