/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>

#include "storage_room.h"
#include "storage_test.h"

constexpr uint32_t STORAGE_TEST_GROWS = 5;

static int reopen(storage_test_t *t)
{
    (void)bridge_destroy(t->w);
    t->w = bridge_create(&t->cfg, storage_test_hear, t);
    return (t->w == nullptr);
}

enum storage_test_end storage_test_size_hash(storage_test_t *t)
{
    t->cfg.table_hash_fill = (uint32_t)atoi(t->more);
    if (reopen(t))
        return (STORAGE_TEST_FAILED);
    printf("hash room %u for %u table ids\n", t->w->storage->hash_room,
        t->w->storage->table_room + 1);
    return (STORAGE_TEST_DONE);
}

enum storage_test_end storage_test_grow_by_share(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(ONE_MORE_PATH, 1, 4096, 4096);
    uint32_t i = 0;

    t->cfg.table_growth = (uint32_t)atoi(t->more);
    if (reopen(t) || bridge_load(t->w, &a))
        return (STORAGE_TEST_FAILED);
    printf("rooms:");
    while (i < STORAGE_TEST_GROWS) {
        (void)bridge_run_phase(t->w, "spawn");
        printf(" %u", t->w->storage->tables[1].room);
        i++;
    }
    printf("\n");
    return (STORAGE_TEST_DONE);
}

enum storage_test_end storage_test_ask_less(storage_test_t *t)
{
    uint32_t rows = 6;

    t->cfg.table_retry = (uint32_t)atoi(t->more);
    if (reopen(t))
        return (STORAGE_TEST_FAILED);
    printf("asks from %u rows:", rows);
    while (rows) {
        rows = get_retry(t->w->storage, rows);
        printf(" %u", rows);
    }
    printf("\n");
    return (STORAGE_TEST_DONE);
}
