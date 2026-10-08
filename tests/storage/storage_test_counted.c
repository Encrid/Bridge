/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/storage/storage_test_counted.c -- the allocator of the build that
** counts: the library is compiled with its calls to malloc, calloc, realloc
** and free sent here, and one of those that ask for memory can be made to
** answer that there is none.
**
** failure.md, *Out of memory*
*/

#include <stdlib.h>

#include "storage_test.h"

#ifdef COUNTED

/*
** count_request
**
** Counts a call that asks for memory. Answers whether it is the one that
** is to be answered null, and keeps that one was.
*/
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

/*
** counted_malloc
**
** malloc, counted: nullptr if this call is the one that is to fail.
*/
void *counted_malloc(size_t n)
{
    if (count_request())
        return (nullptr);
    return (malloc(n));
}

/*
** counted_calloc
**
** calloc, counted: nullptr if this call is the one that is to fail.
*/
void *counted_calloc(size_t n, size_t each)
{
    if (count_request())
        return (nullptr);
    return (calloc(n, each));
}

/*
** counted_realloc
**
** realloc, counted: nullptr if this call is the one that is to fail, and
** what it was handed is then as it was.
*/
void *counted_realloc(void *p, size_t n)
{
    if (count_request())
        return (nullptr);
    return (realloc(p, n));
}

/*
** counted_free
**
** free, counted.
*/
void counted_free(void *p)
{
    storage_test_get_allocator()->calls++;
    free(p);
}

#endif
