/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "storage_test.h"

static bridge_artifact_t state_share(const char *identity, int64_t persistent)
{
    bridge_artifact_t a = storage_test_state(OK_PATH, BRIDGE_NONE, 1 << 19,
        BRIDGE_NONE);

    a.cfg.identity = identity;
    a.cfg.persistent = persistent;
    return (a);
}

enum storage_test_end storage_test_count_ranges(storage_test_t *t)
{
    bridge_artifact_t a = state_share("st:a@0.1.0", 1);
    bridge_artifact_t n = state_share("st:b@0.1.0", 8);
    uint32_t loaded;
    int refused;

    storage_test_load_reloadable(t->w, &a);
    if (storage_test_load(t, &n))
        return (STORAGE_TEST_FAILED);
    loaded = t->w->arena_ranges;
    a.cfg.persistent = 5;
    refused = bridge_reload(t->w, &a, 1);
    printf("ranges: %u when loaded, reload %s, %u after it", loaded,
        refused ? "refused" : "done", t->w->arena_ranges);
    (void)bridge_unload(t->w, "st:a@0.1.0");
    (void)bridge_unload(t->w, "st:b@0.1.0");
    printf(", %u after the unloads\n", t->w->arena_ranges);
    return (STORAGE_TEST_DONE);
}
