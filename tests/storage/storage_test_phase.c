/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "storage_test.h"

static const char *const POLICIES[] = { "", "strip", "sweep", "delete" };

storage_test_allocator_t *storage_test_get_allocator(void)
{
    static storage_test_allocator_t allocator = {};

    return (&allocator);
}

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

void storage_test_run_all(storage_test_t *t, const char *const *phases)
{
    size_t i = 0;

    while (phases[i]) {
        storage_test_run(t, phases[i]);
        i++;
    }
}

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
