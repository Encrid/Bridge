/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "os_types.h"

os_mutex_t *os_mutex_create(void)
{
    os_mutex_t *m = malloc(sizeof(*m));

    if (!m)
        return (nullptr);
    InitializeCriticalSection(&m->section);
    return (m);
}

void os_mutex_free(os_mutex_t *m)
{
    if (!m)
        return;
    DeleteCriticalSection(&m->section);
    free(m);
}

void os_mutex_lock(os_mutex_t *m)
{
    EnterCriticalSection(&m->section);
}

void os_mutex_unlock(os_mutex_t *m)
{
    LeaveCriticalSection(&m->section);
}
