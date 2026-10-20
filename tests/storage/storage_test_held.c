/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#ifdef _WIN32
    #include <windows.h>
    #include <psapi.h>
#endif

#include "storage_test.h"

#ifdef _WIN32

void storage_test_say_held(const bridge_world_t *w)
{
    PROCESS_MEMORY_COUNTERS c = { .cb = sizeof(c) };

    if (!K32GetProcessMemoryInfo(GetCurrentProcess(), &c, sizeof(c)))
        return;
    printf("columns in use %u KB, and the process holds %zu KB\n",
        w->column_space.used >> 10, (size_t)c.WorkingSetSize >> 10);
}

#else

void storage_test_say_held(const bridge_world_t *w)
{
    printf("columns in use %u KB\n", w->column_space.used >> 10);
}

#endif
