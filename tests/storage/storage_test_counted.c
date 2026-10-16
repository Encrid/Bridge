/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "storage_test.h"

#ifdef COUNTED

static bool count_request(void)
{
    storage_test_allocator_t *allocator = storage_test_get_allocator();

    allocator->asked++;
    allocator->calls++;
    if (allocator->asked != allocator->fail_at)
        return (false);
    allocator->failed = true;
    return (true);
}

void *counted_malloc(size_t n)
{
    if (count_request())
        return (nullptr);
    return (malloc(n));
}

void *counted_calloc(size_t n, size_t each)
{
    if (count_request())
        return (nullptr);
    return (calloc(n, each));
}

void *counted_realloc(void *p, size_t n)
{
    if (count_request())
        return (nullptr);
    return (realloc(p, n));
}

void counted_free(void *p)
{
    storage_test_get_allocator()->calls++;
    free(p);
}

#endif
