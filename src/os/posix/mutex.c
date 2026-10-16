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
    if (!pthread_mutex_init(&m->mutex, nullptr))
        return (m);
    free(m);
    return (nullptr);
}

void os_mutex_free(os_mutex_t *m)
{
    if (!m)
        return;
    pthread_mutex_destroy(&m->mutex);
    free(m);
}

void os_mutex_lock(os_mutex_t *m)
{
    pthread_mutex_lock(&m->mutex);
}

void os_mutex_unlock(os_mutex_t *m)
{
    pthread_mutex_unlock(&m->mutex);
}
