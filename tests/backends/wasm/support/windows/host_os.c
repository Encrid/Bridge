/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <windows.h>

#include "host.h"

static DWORD WINAPI run_thread(LPVOID arg)
{
    host_thread_t *t = arg;

    t->run(t->arg);
    return (0);
}

void host_thread_start(host_thread_t *t)
{
    t->handle = CreateThread(nullptr, 0, run_thread, t, 0, nullptr);
}

void host_thread_join(host_thread_t *t)
{
    WaitForSingleObject(t->handle, INFINITE);
    CloseHandle(t->handle);
}
