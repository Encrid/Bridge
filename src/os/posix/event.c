/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <time.h>

#include "os_types.h"

os_event_t *os_event_create(void)
{
    os_event_t *e = malloc(sizeof(*e));

    if (!e)
        return (nullptr);
    e->set = 0;
    if (pthread_mutex_init(&e->lock, nullptr)) {
        free(e);
        return (nullptr);
    }
    if (!pthread_cond_init(&e->cond, nullptr))
        return (e);
    pthread_mutex_destroy(&e->lock);
    free(e);
    return (nullptr);
}

void os_event_free(os_event_t *e)
{
    if (!e)
        return;
    pthread_cond_destroy(&e->cond);
    pthread_mutex_destroy(&e->lock);
    free(e);
}

void os_event_set(os_event_t *e)
{
    pthread_mutex_lock(&e->lock);
    e->set = 1;
    pthread_cond_signal(&e->cond);
    pthread_mutex_unlock(&e->lock);
}

void os_event_wait(os_event_t *e, uint64_t nanos)
{
    struct timespec ts;
    uint64_t total;

    pthread_mutex_lock(&e->lock);
    if (nanos == OS_FOREVER) {
        while (!e->set)
            pthread_cond_wait(&e->cond, &e->lock);
    } else if (!e->set) {
        clock_gettime(CLOCK_REALTIME, &ts);
        total = (uint64_t)ts.tv_nsec + nanos;
        ts.tv_sec += (time_t)(total / OS_SECOND);
        ts.tv_nsec = (long)(total % OS_SECOND);
        pthread_cond_timedwait(&e->cond, &e->lock, &ts);
    }
    e->set = 0;
    pthread_mutex_unlock(&e->lock);
}
