/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "storage_test.h"

#ifdef COUNTED

static bool relay_one(storage_test_t *t, size_t k)
{
    storage_test_allocator_t *allocator = storage_test_get_allocator();
    bridge_artifact_t a = storage_test_state(LAYOUT0_PATH, 8, 1 << 20, 4096);
    bool empty = !strcmp(t->what, "nomemoryrelayempty");

    allocator->asked = 0;
    allocator->fail_at = k;
    allocator->failed = false;
    t->said_memory = 0;
    t->w = bridge_create(&t->cfg, storage_test_hear_memory, t);
    if (!t->w)
        return (allocator->failed);
    a.cfg.reloadable = BRIDGE_TRUE;
    a.cfg.hot_reload = BRIDGE_TRUE;
    if (!bridge_load(t->w, &a) && (empty || !bridge_run_phase(t->w, "spawn"))) {
        a.path = empty ? LAYOUT15_PATH : LAYOUT1_PATH;
        (void)bridge_reload(t->w, &a, 1);
    }
    (void)bridge_destroy(t->w);
    return (allocator->failed);
}

enum storage_test_end storage_test_refuse_relay(storage_test_t *t)
{
    size_t k = 1;
    size_t silent = 0;

    (void)bridge_destroy(t->w);
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
