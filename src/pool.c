/* The threads a phase runs on: `threads - 1` workers kept for the bridge's life, and the caller.
 * docs/bridge/run/threads.md. Nothing here knows what a job is. */
#include "inside.h"

#include <stdlib.h>
#include <string.h>

typedef struct { pool *p; os_event *wake; os_thread *thread; } worker;

struct pool {
    worker    *workers;
    uint32_t   worker_count;
    os_mutex  *lock;
    os_event  *done;
    int        quit;
    /* the run in progress: all of it is read and written under the lock */
    pool_fn        fn;
    void         **jobs;
    const uint8_t *caller_only;
    uint8_t       *taken;
    uint32_t       count, left, taken_room;
};

/* the next job nobody took, or -1. A job marked for the caller is the caller's alone. */
static int take(pool *p, int caller)
{
    uint32_t i;
    int found = -1;
    os_mutex_lock(p->lock);
    for (i = 0; i < p->count && found < 0; i ++) {
        if (!p->taken[i] && (caller || !p->caller_only[i])) {
            p->taken[i] = 1;
            found = (int)i;
        }
    }
    os_mutex_unlock(p->lock);
    return found;
}

static void finished(pool *p)
{
    os_mutex_lock(p->lock);
    if (!-- p->left) os_event_set(p->done);
    os_mutex_unlock(p->lock);
}

static void work(void *arg)
{
    worker *w = arg;
    pool *p = w->p;
    for (;;) {
        int job;
        os_event_wait(w->wake, OS_FOREVER);
        if (p->quit) return;
        while ((job = take(p, 0)) >= 0) {
            p->fn(p->jobs[job]);
            finished(p);
        }
    }
}

/* nullptr if there was not the memory for it: failure.md, *Out of memory*. What was made of
 * it by then is unmade, a worker at a time, since each is counted only once it runs. */
pool *pool_create(uint32_t workers, uint32_t most_jobs)
{
    pool *p = calloc(1, sizeof *p);
    uint32_t i;
    if (!p) return nullptr;
    p->taken = calloc(most_jobs ? most_jobs : 1, 1);
    p->lock = os_mutex_new();
    p->done = os_event_new();
    p->workers = calloc(workers ? workers : 1, sizeof *p->workers);
    if (!p->taken || !p->lock || !p->done || !p->workers) {
        pool_destroy(p);
        return nullptr;
    }
    for (i = 0; i < workers; i ++) {
        p->workers[i].p = p;
        p->workers[i].wake = os_event_new();
        p->workers[i].thread = p->workers[i].wake ? os_thread_start(work, &p->workers[i]) : nullptr;
        if (!p->workers[i].thread) {
            os_event_free(p->workers[i].wake);
            pool_destroy(p);
            return nullptr;
        }
        p->worker_count = i + 1;
    }
    return p;
}

void pool_destroy(pool *p)
{
    uint32_t i;
    if (!p) return;
    p->quit = 1;
    for (i = 0; i < p->worker_count; i ++) {
        os_event_set(p->workers[i].wake);
        os_thread_join(p->workers[i].thread);
        os_event_free(p->workers[i].wake);
    }
    os_event_free(p->done);
    os_mutex_free(p->lock);
    free(p->workers);
    free(p->taken);
    free(p);
}

void pool_run(pool *p, pool_fn fn, void **jobs, const uint8_t *caller_only, uint32_t count)
{
    uint32_t i;
    int job, left;
    /* one job, or nobody to share with: no thread is woken for it */
    if (count < 2 || !p->worker_count) {
        for (i = 0; i < count; i ++) fn(jobs[i]);
        return;
    }
    os_mutex_lock(p->lock);
    memset(p->taken, 0, count);
    p->fn = fn;
    p->jobs = jobs;
    p->caller_only = caller_only;
    p->count = p->left = count;
    os_mutex_unlock(p->lock);

    for (i = 0; i < p->worker_count && i + 1 < count; i ++) os_event_set(p->workers[i].wake);
    while ((job = take(p, 1)) >= 0) {
        fn(jobs[job]);
        finished(p);
    }
    do {
        os_event_wait(p->done, OS_FOREVER);
        os_mutex_lock(p->lock);
        left = (int)p->left;
        os_mutex_unlock(p->lock);
    } while (left);
    /* nothing of this run may be taken once the caller's arrays are gone */
    os_mutex_lock(p->lock);
    p->count = 0;
    os_mutex_unlock(p->lock);
}
