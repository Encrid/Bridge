/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "os_types.h"

static DWORD WINAPI run_thread(LPVOID p)
{
    os_thread_t *t = p;

    t->fn(t->arg);
    return (0);
}

os_thread_t *os_thread_start(os_thread_fn_t fn, void *arg)
{
    os_thread_t *t = malloc(sizeof(*t));

    if (!t)
        return (nullptr);
    t->fn = fn;
    t->arg = arg;
    t->handle = CreateThread(nullptr, 0, run_thread, t, 0, nullptr);
    if (t->handle)
        return (t);
    free(t);
    return (nullptr);
}

void os_thread_join(os_thread_t *t)
{
    WaitForSingleObject(t->handle, INFINITE);
    CloseHandle(t->handle);
    free(t);
}

uint64_t os_get_thread_id(void)
{
    return (GetCurrentThreadId());
}

uint64_t os_get_now(void)
{
    LARGE_INTEGER frequency;
    LARGE_INTEGER count;
    uint64_t ticks;
    uint64_t whole;
    uint64_t rest;

    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&count);
    ticks = (uint64_t)frequency.QuadPart;
    whole = (uint64_t)count.QuadPart / ticks;
    rest = (uint64_t)count.QuadPart % ticks;
    return (whole * OS_SECOND + rest * OS_SECOND / ticks);
}
