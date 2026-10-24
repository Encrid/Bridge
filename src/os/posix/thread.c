/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
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

os_thread_t *os_thread_start(os_thread_fn_t fn, void *arg)
{
    os_thread_t *t = malloc(sizeof(*t));

    if (!t)
        return (nullptr);
    t->fn = fn;
    t->arg = arg;
    if (!pthread_create(&t->handle, nullptr, run_thread, t))
        return (t);
    free(t);
    return (nullptr);
}

void os_thread_join(os_thread_t *t)
{
    pthread_join(t->handle, nullptr);
    free(t);
}

uint64_t os_get_thread_id(void)
{
    return ((uint64_t)(uintptr_t)pthread_self());
}

uint64_t os_get_now(void)
{
    struct timespec t;

    clock_gettime(CLOCK_MONOTONIC_RAW, &t);
    return ((uint64_t)t.tv_sec * OS_SECOND + (uint64_t)t.tv_nsec);
}
