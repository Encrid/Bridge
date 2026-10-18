/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "pool.h"

static int start_worker(pool_t *p, uint32_t i)
{
    pool_worker_t *w = &p->workers[i];

    w->p = p;
    w->wake = os_event_create();
    if (!w->wake)
        return (-1);
    w->thread = os_thread_start(pool_work, w);
    if (!w->thread) {
        os_event_free(w->wake);
        return (-1);
    }
    p->worker_count = i + 1;
    return (0);
}

pool_t *pool_create(uint32_t workers, uint32_t most_jobs)
{
    pool_t *p = calloc(1, sizeof(*p));
    uint32_t i = 0;

    if (!p)
        return (nullptr);
    p->taken = calloc(most_jobs ? most_jobs : 1, 1);
    p->lock = os_mutex_create();
    p->done = os_event_create();
    p->workers = calloc(workers ? workers : 1, sizeof(*p->workers));
    if (!p->taken || !p->lock || !p->done || !p->workers)
        i = workers + 1;
    while (i < workers && !start_worker(p, i))
        i++;
    if (i != workers) {
        pool_destroy(p);
        return (nullptr);
    }
    return (p);
}

void pool_destroy(pool_t *p)
{
    uint32_t i = 0;

    if (!p)
        return;
    p->quit = 1;
    while (i < p->worker_count) {
        os_event_set(p->workers[i].wake);
        os_thread_join(p->workers[i].thread);
        os_event_free(p->workers[i].wake);
        i++;
    }
    os_event_free(p->done);
    os_mutex_free(p->lock);
    free(p->workers);
    free(p->taken);
    free(p);
}
