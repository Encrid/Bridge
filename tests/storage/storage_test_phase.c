/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/storage/storage_test_phase.c -- one phase and its barrier, named
** in the transcript, and the count of the allocator's calls that a phase
** must not raise.
**
** budgets.md, *Nothing is allocated in a phase*
*/

#include <stdio.h>
#include <string.h>

#include "storage_test.h"

/*
** What a transcript calls each policy, by its number.
*/
static const char *const POLICIES[] = { "", "strip", "sweep", "delete" };

/*
** storage_test_get_allocator
**
** Answers the count of the library's calls to the allocator. In the build
** that does not count them it is all zeros, and stays so. It is a variable
** of this function because the allocator's functions are handed nothing a
** fixture could be reached through.
*/
storage_test_allocator_t *storage_test_get_allocator(void)
{
    static storage_test_allocator_t allocator = {};

    return (&allocator);
}

/*
** storage_test_run
**
** Runs one phase and its barrier, named in the transcript. A phase that is
** refused is said, and so is one in which the library called the
** allocator: built with COUNTED, a phase with its barrier is to make no
** call.
*/
void storage_test_run(storage_test_t *t, const char *phase)
{
    const storage_test_allocator_t *allocator = storage_test_get_allocator();
    size_t before = allocator->calls;

    printf("%s\n", phase);
    if (bridge_run_phase(t->b, phase))
        printf("  REFUSED\n");
    if (allocator->calls != before)
        printf("  FAIL: the allocator was called %u times in a phase\n",
            (unsigned)(allocator->calls - before));
}

/*
** storage_test_run_all
**
** Runs each phase of a list that ends with nullptr, as storage_test_run
** does.
*/
void storage_test_run_all(storage_test_t *t, const char *const *phases)
{
    size_t i = 0;

    while (phases[i]) {
        storage_test_run(t, phases[i]);
        i++;
    }
}

/*
** storage_test_drop
**
** What unload does to a component with this policy, asked of storage
** directly so that the module is still there to look; and then a barrier,
** for what was heard and for what a module reads.
**
** lifetime.md, *What unload leaves behind*
*/
void storage_test_drop(storage_test_t *t, const char *name,
    enum bridge_on_unload policy)
{
    bridge_t *b = t->b;
    uint32_t i = 0;

    printf("%s %s\n", POLICIES[policy], name);
    while (i < b->type_count) {
        if (!strcmp(b->types[i]->name, name))
            storage_drop(b->storage, b->types[i]->type, policy);
        i++;
    }
    storage_test_run(t, "never");
}
