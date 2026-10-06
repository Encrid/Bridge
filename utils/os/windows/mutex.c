/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** utils/os/windows/mutex.c -- a mutex on Windows: a critical section.
*/

#include <stdlib.h>

#include "os_types.h"

/*
** os_mutex_create
**
** Makes a mutex. nullptr if there is no memory for it.
*/
os_mutex_t *os_mutex_create(void)
{
    os_mutex_t *m = malloc(sizeof(*m));

    if (!m)
        return (nullptr);
    InitializeCriticalSection(&m->section);
    return (m);
}

/*
** os_mutex_free
**
** Frees a mutex nobody holds. A null `m` is nothing to free.
*/
void os_mutex_free(os_mutex_t *m)
{
    if (!m)
        return;
    DeleteCriticalSection(&m->section);
    free(m);
}

/*
** os_mutex_lock
**
** Takes a mutex, waiting for whoever holds it.
*/
void os_mutex_lock(os_mutex_t *m)
{
    EnterCriticalSection(&m->section);
}

/*
** os_mutex_unlock
**
** Gives a mutex back.
*/
void os_mutex_unlock(os_mutex_t *m)
{
    LeaveCriticalSection(&m->section);
}
