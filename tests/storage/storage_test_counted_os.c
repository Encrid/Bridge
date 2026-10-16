/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "storage_test.h"

#ifdef COUNTED
    #ifdef _WIN32
        #include <windows.h>

HANDLE counted_create_event(LPSECURITY_ATTRIBUTES attributes, BOOL manual,
    BOOL initial, LPCSTR name)
{
    if (storage_test_count_request())
        return (nullptr);
    return (CreateEventA(attributes, manual, initial, name));
}

HANDLE counted_create_thread(LPSECURITY_ATTRIBUTES attributes, SIZE_T stack,
    LPTHREAD_START_ROUTINE start, LPVOID arg, DWORD flags, LPDWORD id)
{
    if (storage_test_count_request())
        return (nullptr);
    return (CreateThread(attributes, stack, start, arg, flags, id));
}

    #else
        #include <errno.h>
        #include <pthread.h>

int counted_pthread_create(pthread_t *thread, const pthread_attr_t *attributes,
    void *(*start)(void *), void *arg)
{
    if (storage_test_count_request())
        return (EAGAIN);
    return (pthread_create(thread, attributes, start, arg));
}

int counted_mutex_init(pthread_mutex_t *mutex,
    const pthread_mutexattr_t *attributes)
{
    if (storage_test_count_request())
        return (ENOMEM);
    return (pthread_mutex_init(mutex, attributes));
}

int counted_cond_init(pthread_cond_t *cond, const pthread_condattr_t *attributes)
{
    if (storage_test_count_request())
        return (ENOMEM);
    return (pthread_cond_init(cond, attributes));
}

    #endif
#endif
