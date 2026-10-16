/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** utils/os/posix/event.c -- an event on Linux and macOS: a flag, the mutex
** that guards it and the condition a wait sleeps on.
*/

#include <errno.h>
#include <stdlib.h>
#include <time.h>

#include "os_types.h"

/*
** os_event_create
**
** Makes an event, not set. nullptr if there is no memory for it.
*/
os_event_t *os_event_create(void)
{
    os_event_t *e = malloc(sizeof(*e));

    if (!e)
        return (nullptr);
    pthread_mutex_init(&e->lock, nullptr);
    pthread_cond_init(&e->cond, nullptr);
    e->set = 0;
    return (e);
}

/*
** os_event_free
**
** Frees an event nobody waits for. A null `e` is nothing to free.
*/
void os_event_free(os_event_t *e)
{
    if (!e)
        return;
    pthread_cond_destroy(&e->cond);
    pthread_mutex_destroy(&e->lock);
    free(e);
}

/*
** os_event_set
**
** Sets an event: one wait, now or later, returns for it.
*/
void os_event_set(os_event_t *e)
{
    pthread_mutex_lock(&e->lock);
    e->set = 1;
    pthread_cond_signal(&e->cond);
    pthread_mutex_unlock(&e->lock);
}

/*
** get_deadline
**
** Answers when a wait of `nanos` from now ends, by the clock a condition
** counts in.
*/
static struct timespec get_deadline(uint64_t nanos)
{
    struct timespec until;

    clock_gettime(CLOCK_REALTIME, &until);
    until.tv_sec += (time_t)(nanos / OS_SECOND);
    until.tv_nsec += (long)(nanos % OS_SECOND);
    if (until.tv_nsec >= (long)OS_SECOND) {
        until.tv_sec++;
        until.tv_nsec -= (long)OS_SECOND;
    }
    return (until);
}

/*
** os_event_wait
**
** Waits for an event for at most `nanos`, or until it is set if that is
** OS_FOREVER. A wait that wakes with the event not set waits again. 1 if it
** was set, and it no longer is; 0 if the time ran out.
*/
int os_event_wait(os_event_t *e, uint64_t nanos)
{
    struct timespec until = {};
    int ended = 0;
    int got;

    if (nanos != OS_FOREVER)
        until = get_deadline(nanos);
    pthread_mutex_lock(&e->lock);
    while (!e->set && ended != ETIMEDOUT && nanos != OS_FOREVER)
        ended = pthread_cond_timedwait(&e->cond, &e->lock, &until);
    while (!e->set && nanos == OS_FOREVER)
        pthread_cond_wait(&e->cond, &e->lock);
    got = e->set;
    e->set = 0;
    pthread_mutex_unlock(&e->lock);
    return (got);
}
