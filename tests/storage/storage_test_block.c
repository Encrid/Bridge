/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "block.h"
#include "storage_test.h"

enum storage_test_end storage_test_block_offset([[maybe_unused]] storage_test_t *t)
{
    uint8_t buf[256] = {};
    bridge_world_t w = { .shared = buf + 100, .shared_len = 100 };

    printf("offset null %u below %u above %u inside %u\n",
        block_get_offset(&w, nullptr), block_get_offset(&w, buf + 50),
        block_get_offset(&w, buf + 250), block_get_offset(&w, buf + 150));
    return (STORAGE_TEST_LEFT);
}

enum storage_test_end storage_test_index_outside(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(STORE_PATH, 6, 1 << 20, 1 << 20);
    size_t saved = t->w->shared_len;

    a.cfg.on_error = BRIDGE_SKIP;
    t->w->shared_len = 0;
    (void)storage_test_try_load(t, &a);
    t->w->shared_len = saved;
    return (STORAGE_TEST_DONE);
}
