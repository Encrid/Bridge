/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** utils/os/posix/thread.c -- a thread on Linux and macOS, and the clock.
*/

#include <stdlib.h>
#include <time.h>

#include "os_types.h"

static void *run_thread(void *p)
{
    os_thread_t *t = p;

    t->fn(t->arg);
    return (nullptr);
}

/*
** os_thread_start
**
** Starts a thread that runs `fn(arg)`. nullptr if there is no memory for it.
*/
os_thread_t *os_thread_start(os_thread_fn_t fn, void *arg)
{
    os_thread_t *t = malloc(sizeof(*t));

    if (!t)
        return (nullptr);
    t->fn = fn;
    t->arg = arg;
    pthread_create(&t->handle, nullptr, run_thread, t);
    return (t);
}

/*
** os_thread_join
**
** Waits for a thread to end, and frees what os_thread_start answered.
*/
void os_thread_join(os_thread_t *t)
{
    pthread_join(t->handle, nullptr);
    free(t);
}

/*
** os_get_thread_id
**
** Answers the calling thread's handle as a number, which is never 0 for a
** thread that runs.
*/
uint64_t os_get_thread_id(void)
{
    return ((uint64_t)(uintptr_t)pthread_self());
}

/*
** os_get_now
**
** Answers nanoseconds from the monotonic clock.
*/
uint64_t os_get_now(void)
{
    struct timespec t;

    clock_gettime(CLOCK_MONOTONIC, &t);
    return ((uint64_t)t.tv_sec * OS_SECOND + (uint64_t)t.tv_nsec);
}
