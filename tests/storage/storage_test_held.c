/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/storage/storage_test_held.c -- what the process holds, asked of
** the operating system. The figures of the plan are taken on Windows, and
** elsewhere this says nothing of the process.
*/

#include <stdio.h>

#ifdef _WIN32
    #include <windows.h>
    #include <psapi.h>
#endif

#include "storage_test.h"

#ifdef _WIN32

/*
** storage_test_say_held
**
** Says the columns in use and what the process holds, in KB: the touched
** pages of the block and everything outside it. Says nothing if the
** operating system does not answer.
*/
void storage_test_say_held(const bridge_t *b)
{
    PROCESS_MEMORY_COUNTERS c = { .cb = sizeof(c) };

    if (!K32GetProcessMemoryInfo(GetCurrentProcess(), &c, sizeof(c)))
        return;
    printf("columns in use %u KB, and the process holds %zu KB\n",
        b->column_space.used >> 10, (size_t)c.WorkingSetSize >> 10);
}

#else

/*
** storage_test_say_held
**
** Says the columns in use, in KB.
*/
void storage_test_say_held(const bridge_t *b)
{
    printf("columns in use %u KB\n", b->column_space.used >> 10);
}

#endif
