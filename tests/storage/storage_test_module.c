/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>

#include "storage_test.h"

bridge_artifact_t storage_test_state(const char *path, int64_t spawns,
    uint32_t spans, int64_t changes)
{
    bridge_artifact_t a = { .backend = "tests", .path = path };

    a.cfg = (bridge_module_config_t){ .identity = "st:store@0.1.0",
        .scratch = 64 << 10, .abi = BRIDGE_ABI,
        .reloadable = BRIDGE_FALSE, .hot_reload = BRIDGE_FALSE,
        .pinned = BRIDGE_FALSE, .on_error = BRIDGE_FAIL,
        .max_system_failures = 1, .work_limit = BRIDGE_NO_LIMIT,
        .memory_limit = BRIDGE_NO_LIMIT, .init_timeout = BRIDGE_NO_LIMIT,
        .reload_timeout = BRIDGE_NO_LIMIT,
        .shutdown_timeout = BRIDGE_NO_LIMIT, .log_ring = 256 * 32,
        .spans = spans, .log_level = BRIDGE_INFO, .spawns = spawns,
        .persistent = BRIDGE_NONE, .changes = changes,
        .loss_count = BRIDGE_NONE, .config = BRIDGE_NO_CONFIG };
    return (a);
}

int storage_test_load(storage_test_t *t, const bridge_artifact_t *a)
{
    if (!bridge_load(t->b, a))
        return (0);
    printf("FAIL: not loaded\n");
    return (1);
}

int storage_test_try_load(storage_test_t *t, const bridge_artifact_t *a)
{
    if (!bridge_load(t->b, a))
        return (0);
    printf("not loaded\n");
    return (1);
}

void storage_test_load_reloadable(bridge_t *b, bridge_artifact_t *a)
{
    a->cfg.reloadable = BRIDGE_TRUE;
    a->cfg.hot_reload = BRIDGE_TRUE;
    if (b && !bridge_load(b, a))
        return;
    printf("FAIL: not loaded\n");
    exit(1);
}

int storage_test_reopen(storage_test_t *t, bridge_sink_fn_t sink,
    const bridge_artifact_t *a)
{
    (void)bridge_destroy(t->b);
    t->b = bridge_create(&t->cfg, sink, t);
    if (t->b && !bridge_load(t->b, a))
        return (0);
    printf("FAIL: not loaded\n");
    return (1);
}
