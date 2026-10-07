/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/storage/storage_test_nomemory.c -- the allocator answering that
** there is no memory, at every call the library makes in turn. Of the
** build that counts those calls, and of no other.
**
** failure.md, *Out of memory*
*/

#include <stdio.h>

#include "storage_test.h"

#ifdef COUNTED

static void use_module(bridge_t *b, const bridge_artifact_t *a)
{
    (void)bridge_run_phase(b, "spawn");
    (void)bridge_run_phase(b, "attach_a");
    (void)bridge_run_phase(b, "attach_b_half");
    (void)bridge_reload(b, a);
    (void)bridge_run_phase(b, "look");
    (void)bridge_unload(b, a->cfg.identity);
}

/*
** refuse_one
**
** A deck made, a module loaded, run, reloaded and unloaded, and the
** deck destroyed, with the allocation numbered `k` answered null. Answers
** whether there was such an allocation. With an argument after the case's
** name, which allocation it is goes to stderr first, to find a crash.
*/
static bool refuse_one(storage_test_t *t, size_t k)
{
    storage_test_allocator_t *allocator = storage_test_get_allocator();
    bridge_artifact_t a = storage_test_state(NSTORE_PATH, 6, 1 << 20, 1 << 20);

    if (t->more)
        fprintf(stderr, "allocation %u\n", (unsigned)k);
    allocator->asked = 0;
    allocator->fail_at = k;
    allocator->failed = false;
    t->said_memory = 0;
    t->b = bridge_create(&t->cfg, storage_test_hear_memory, t);
    if (!t->b)
        return (allocator->failed);
    a.cfg.reloadable = BRIDGE_TRUE;
    a.cfg.hot_reload = BRIDGE_TRUE;
    if (!bridge_load(t->b, &a))
        use_module(t->b, &a);
    (void)bridge_destroy(t->b);
    return (allocator->failed);
}

/*
** storage_test_refuse_memory
**
** Once for every allocation that a deck and a module make from the first
** call to the last, with that one answered null. Each time something is
** refused and says why, and nothing is written through a null pointer. An
** allocation whose refusal no record said is counted, and with an argument
** after the case's name is said on stderr.
*/
enum storage_test_end storage_test_refuse_memory(storage_test_t *t)
{
    size_t k = 1;
    size_t silent = 0;

    (void)bridge_destroy(t->b);
    while (refuse_one(t, k)) {
        silent += !t->said_memory;
        if (t->more && !t->said_memory)
            fprintf(stderr, "unsaid\n");
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
