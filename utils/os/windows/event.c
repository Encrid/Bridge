/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** utils/os/windows/event.c -- an event on Windows: one that resets itself
** when a wait returns for it.
*/

#include <stdlib.h>

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
    e->handle = CreateEventA(nullptr, FALSE, FALSE, nullptr);
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
    CloseHandle(e->handle);
    free(e);
}

/*
** os_event_set
**
** Sets an event: one wait, now or later, returns for it.
*/
void os_event_set(os_event_t *e)
{
    SetEvent(e->handle);
}

/*
** os_event_wait
**
** Waits for an event for at most `nanos`, rounded up to a millisecond, which
** is what Windows counts in. 1 if it was set, 0 if the time ran out.
*/
int os_event_wait(os_event_t *e, uint64_t nanos)
{
    DWORD ms = INFINITE;

    if (nanos != OS_FOREVER)
        ms = (DWORD)((nanos + OS_MILLISECOND - 1) / OS_MILLISECOND);
    return (WaitForSingleObject(e->handle, ms) == WAIT_OBJECT_0);
}
