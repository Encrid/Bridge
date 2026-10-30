/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "storage_test.h"

#ifdef COUNTED

void *counted_os_reserve(size_t reserve, size_t commit)
{
    if (storage_test_count_request())
        return (nullptr);
    return (os_reserve(reserve, commit));
}

int counted_os_commit(void *base, size_t bytes)
{
    if (storage_test_count_request())
        return (-1);
    return (os_commit(base, bytes));
}

os_thread_t *counted_os_thread_start(os_thread_fn_t fn, void *arg,
    size_t stack)
{
    if (storage_test_count_request())
        return (nullptr);
    return (os_thread_start(fn, arg, stack));
}

os_mutex_t *counted_os_mutex_create(void)
{
    if (storage_test_count_request())
        return (nullptr);
    return (os_mutex_create());
}

os_event_t *counted_os_event_create(void)
{
    if (storage_test_count_request())
        return (nullptr);
    return (os_event_create());
}

#endif
