/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "storage_test.h"

static const char *const POLICIES[] = { "", "strip", "sweep", "delete" };

void storage_test_run(storage_test_t *t, const char *phase)
{
    printf("%s\n", phase);
    if (bridge_run_phase(t->w, phase))
        printf("  REFUSED\n");
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
    bridge_world_t *w = t->w;
    uint32_t i = 0;

    printf("%s %s\n", POLICIES[policy], name);
    while (i < w->type_count) {
        if (!strcmp(w->types[i]->name, name))
            storage_drop(w->storage, w->types[i]->type, policy);
        i++;
    }
    storage_test_run(t, "never");
}

void storage_test_say_alive(const storage_test_t *t, uint32_t last)
{
    uint32_t e = 1;
    bool alive;

    while (e <= last) {
        alive = storage_is_alive(t->w->storage, (uint64_t)1 << 32 | e);
        printf("entity %u is %s\n", e, alive ? "alive" : "gone");
        e++;
    }
}
