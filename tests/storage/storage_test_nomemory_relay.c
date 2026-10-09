/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/storage/storage_test_nomemory_relay.c -- a reload into another
** component layout, with each allocation answered null in turn, and what
** the operating system reserves counted among them.
**
** failure.md, *Out of memory*
*/

#include <stdio.h>

#include "os.h"
#include "storage_test.h"

#ifdef COUNTED

/*
** counted_os_reserve
**
** os_reserve, counted among the allocations: nullptr if this call is the
** one that is to fail.
*/
void *counted_os_reserve(size_t reserve, size_t commit)
{
    storage_test_allocator_t *allocator = storage_test_get_allocator();

    allocator->asked++;
    if (allocator->asked != allocator->fail_at)
        return (os_reserve(reserve, commit));
    allocator->failed = true;
    return (nullptr);
}

/*
** relay_one
**
** A deck made, a module loaded, run, reloaded into a build with a field
** added and looked at, with the allocation numbered `k` answered null.
** Answers whether there was such an allocation.
*/
static bool relay_one(storage_test_t *t, size_t k)
{
    storage_test_allocator_t *allocator = storage_test_get_allocator();
    bridge_artifact_t a = storage_test_state(LAYOUT0_PATH, 8, 1 << 20, 4096);

    allocator->asked = 0;
    allocator->fail_at = k;
    allocator->failed = false;
    t->said_memory = 0;
    t->b = bridge_create(&t->cfg, storage_test_hear_memory, t);
    if (!t->b)
        return (allocator->failed);
    a.cfg.reloadable = BRIDGE_TRUE;
    a.cfg.hot_reload = BRIDGE_TRUE;
    if (!bridge_load(t->b, &a) && !bridge_run_phase(t->b, "spawn")) {
        a.path = LAYOUT1_PATH;
        (void)bridge_reload(t->b, &a);
    }
    (void)bridge_destroy(t->b);
    return (allocator->failed);
}

/*
** storage_test_refuse_relay
**
** Once for every allocation of a reload that lays a component out again,
** with that one answered null: each time something is refused and says
** why. With an argument after the case's name, an allocation whose refusal
** no record said is named on stderr.
*/
enum storage_test_end storage_test_refuse_relay(storage_test_t *t)
{
    size_t k = 1;
    size_t silent = 0;

    (void)bridge_destroy(t->b);
    while (relay_one(t, k)) {
        silent += !t->said_memory;
        if (t->more && !t->said_memory)
            fprintf(stderr, "allocation %u unsaid\n", (unsigned)k);
        k++;
    }
    storage_test_get_allocator()->fail_at = 0;
    printf("%s allocations were each answered null in turn\n",
        k > 40 ? "more than forty" : "TOO FEW");
    if (silent)
        printf("FAIL: %u of them were not said\n", (unsigned)silent);
    return (STORAGE_TEST_LEFT);
}

#endif
