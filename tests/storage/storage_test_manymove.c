/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "storage_test.h"

static void say_carried(bridge_t *b, const char *when, uint32_t count)
{
    const declared_type_t *c;
    uint32_t kept = 0;
    uint32_t carried = 0;
    uint32_t i = 0;

    while (i < b->type_count) {
        c = b->types[i];
        if (!strncmp(c->name, "st:M", 4)) {
            kept++;
            carried += storage_count_carriers(b->storage, c->type) == count;
        }
        i++;
    }
    printf("%s: %u types, %u of them carried by each of the %u entities\n",
        when, kept, carried, count);
}

enum storage_test_end storage_test_move_many(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(MANYMOVE1_PATH, 8, 16 << 20, 1 << 20);
    bridge_t *b;

    (void)bridge_destroy(t->b);
    t->cfg.budget.types = 512;
    t->cfg.budget.sparse_sets = 512;
    b = bridge_create(&t->cfg, storage_test_hear, t);
    storage_test_load_reloadable(b, &a);
    (void)bridge_run_phase(b, "spawn");
    say_carried(b, "before the reload", 4);
    a.path = MANYMOVE2_PATH;
    printf("reload: %s\n", bridge_reload(b, &a) ? "refused" : "done");
    say_carried(b, "after the reload", 4);
    (void)bridge_destroy(b);
    return (STORAGE_TEST_LEFT);
}

enum storage_test_end storage_test_grow_many(storage_test_t *t)
{
    bridge_artifact_t a = storage_test_state(MANYMOVE1_PATH, 8, 16 << 20, 1 << 20);
    bridge_t *b;

    (void)bridge_destroy(t->b);
    t->cfg.budget.types = 512;
    b = bridge_create(&t->cfg, storage_test_hear, t);
    storage_test_load_reloadable(b, &a);
    (void)bridge_run_phase(b, "spawn");
    (void)bridge_run_phase(b, "spawn");
    say_carried(b, "after two spawns", 8);
    (void)bridge_destroy(b);
    return (STORAGE_TEST_LEFT);
}
