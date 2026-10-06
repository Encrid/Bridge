/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** utils/os/windows/thread.c -- a thread on Windows, and the clock.
*/

#include <stdlib.h>

#include "os_types.h"

static DWORD WINAPI run_thread(LPVOID p)
{
    os_thread_t *t = p;

    t->fn(t->arg);
    return (0);
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
    t->handle = CreateThread(nullptr, 0, run_thread, t, 0, nullptr);
    return (t);
}

/*
** os_thread_join
**
** Waits for a thread to end, and frees what os_thread_start answered.
*/
void os_thread_join(os_thread_t *t)
{
    WaitForSingleObject(t->handle, INFINITE);
    CloseHandle(t->handle);
    free(t);
}

/*
** os_get_thread_id
**
** Answers the calling thread's id, which Windows never makes 0.
*/
uint64_t os_get_thread_id(void)
{
    return (GetCurrentThreadId());
}

/*
** os_get_now
**
** Answers nanoseconds from the performance counter. The whole seconds and
** the rest are scaled apart, so that the product stays inside 64 bits.
*/
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
