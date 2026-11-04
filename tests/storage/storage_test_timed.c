/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "storage_test.h"

constexpr uint32_t TIMED_TABLES = 64;

static const char *const TIMED_PHASES[] = { "work", "churn", "idle",
    "spawn" };

const char *storage_test_name_timed(char *identity, size_t len, uint32_t i)
{
    snprintf(identity, len, "tm:m%04u@0.1.0", i);
    return (i ? identity : "st:more@0.1.0");
}

int storage_test_load_timed(bridge_world_t *w, uint32_t i)
{
    char identity[32];
    bridge_artifact_t a = storage_test_state(i ? TIMED_PATH : ONE_MORE_PATH, 4,
        4096, 4096);

    a.cfg.identity = storage_test_name_timed(identity, sizeof(identity), i);
    return (bridge_load(w, &a));
}

module_t *storage_test_find_timed(const bridge_world_t *w, uint32_t i)
{
    char identity[32];
    const char *name = storage_test_name_timed(identity, sizeof(identity), i);
    module_t *m = w->modules;

    while (m && strcmp(m->identity, name))
        m = m->next;
    return (m);
}

int storage_test_make_timed(storage_test_t *t, uint32_t modules,
    uint32_t threads)
{
    uint32_t i = 0;

    (void)bridge_destroy(t->w);
    t->cfg.phases = TIMED_PHASES;
    t->cfg.phase_count = sizeof(TIMED_PHASES) / sizeof(*TIMED_PHASES);
    t->cfg.threads = threads;
    t->cfg.budget.modules = modules + 2;
    t->cfg.budget.systems = 2 * modules + 3;
    t->cfg.budget.tables = TIMED_TABLES;
    t->w = bridge_create(&t->cfg, storage_test_hear_nothing, t);
    while (t->w && i <= modules && !storage_test_load_timed(t->w, i))
        i++;
    if (i == modules + 1)
        return (0);
    printf("FAIL: the timed modules were not loaded\n");
    return (1);
}
