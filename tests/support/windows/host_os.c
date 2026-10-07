/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/support/windows/host_os.c -- what a host program asks the
** operating system itself, and nothing of ours: on Windows.
*/

#include <windows.h>

#include "host.h"

/*
** host_is_library_loaded
**
** Answers whether a library is in this process, asked of the operating
** system.
*/
bool host_is_library_loaded(const char *path)
{
    char full[MAX_PATH];
    DWORD len = GetFullPathNameA(path, sizeof(full), full, nullptr);

    return (len && len < sizeof(full) && GetModuleHandleA(full));
}

/*
** host_get_thread_id
**
** Answers which thread this is, as a native test module says it.
*/
uint32_t host_get_thread_id(void)
{
    return ((uint32_t)GetCurrentThreadId());
}

static DWORD WINAPI run_thread(LPVOID arg)
{
    host_thread_t *t = arg;

    t->run(t->arg);
    return (0);
}

/*
** host_thread_start
**
** Starts a thread that calls `t->run` with `t->arg`.
*/
void host_thread_start(host_thread_t *t)
{
    t->handle = CreateThread(nullptr, 0, run_thread, t, 0, nullptr);
}

/*
** host_thread_join
**
** Waits for a thread to end.
*/
void host_thread_join(host_thread_t *t)
{
    WaitForSingleObject(t->handle, INFINITE);
    CloseHandle(t->handle);
}
