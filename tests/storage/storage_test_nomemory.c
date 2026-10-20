/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "storage_test.h"

#ifdef COUNTED

static void use_module(bridge_world_t *w, bridge_artifact_t a)
{
    (void)bridge_run_phase(w, "spawn");
    (void)bridge_run_phase(w, "attach_a");
    (void)bridge_run_phase(w, "attach_b_half");
    a.cfg.spawns = 2;
    a.cfg.persistent = 64;
    (void)bridge_reload(w, &a);
    (void)bridge_run_phase(w, "look");
    (void)bridge_unload(w, a.cfg.identity);
}

static bool refuse_one(storage_test_t *t, size_t k)
{
    storage_test_allocator_t *allocator = storage_test_get_allocator();
    bridge_artifact_t a = storage_test_state(STORE_PATH, 6, 1 << 20, 1 << 20);

    if (t->more)
        fprintf(stderr, "allocation %u\n", (unsigned)k);
    allocator->asked = 0;
    allocator->fail_at = k;
    allocator->failed = false;
    t->said_memory = 0;
    t->w = bridge_create(&t->cfg, storage_test_hear_memory, t);
    if (!t->w)
        return (allocator->failed);
    a.cfg.reloadable = BRIDGE_TRUE;
    a.cfg.hot_reload = BRIDGE_TRUE;
    if (!bridge_load(t->w, &a))
        use_module(t->w, a);
    (void)bridge_destroy(t->w);
    return (allocator->failed);
}

enum storage_test_end storage_test_refuse_memory(storage_test_t *t)
{
    size_t k = 1;
    size_t silent = 0;

    (void)bridge_destroy(t->w);
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
