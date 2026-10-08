/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/storage/storage_test_module.c -- a test module as a deployment
** states it, and the ways a case loads one.
*/

#include <stdio.h>
#include <stdlib.h>

#include "storage_test.h"

/*
** storage_test_state
**
** Answers a trusted native module as a deployment must state it, with the
** identity st:store@0.1.0: one that is not reloadable, and whose first
** failure stops the deck.
*/
bridge_artifact_t storage_test_state(const char *path, int64_t spawns,
    uint32_t spans, int64_t changes)
{
    bridge_artifact_t a = { .backend = "native", .path = path };

    a.cfg = (bridge_module_config_t){ .identity = "st:store@0.1.0",
        .scratch = 64 << 10, .abi = BRIDGE_ABI, .trusted = BRIDGE_TRUE,
        .reloadable = BRIDGE_FALSE, .hot_reload = BRIDGE_FALSE,
        .pinned = BRIDGE_FALSE, .on_error = BRIDGE_FAIL,
        .max_system_failures = 1, .work_limit = BRIDGE_NO_LIMIT,
        .memory_limit = BRIDGE_NO_LIMIT, .init_timeout = BRIDGE_NO_LIMIT,
        .reload_timeout = BRIDGE_NO_LIMIT,
        .shutdown_timeout = BRIDGE_NO_LIMIT, .log_ring = 256 * 24,
        .spans = spans, .log_level = BRIDGE_INFO, .spawns = spawns,
        .persistent = BRIDGE_NONE, .changes = changes,
        .loss_count = BRIDGE_NONE, .config = BRIDGE_NO_CONFIG };
    return (a);
}

/*
** storage_test_load
**
** Loads a module into the fixture's deck. Answers 1, having said "FAIL:
** not loaded", if it was refused.
*/
int storage_test_load(storage_test_t *t, const bridge_artifact_t *a)
{
    if (!bridge_load(t->b, a))
        return (0);
    printf("FAIL: not loaded\n");
    return (1);
}

/*
** storage_test_try_load
**
** Loads a module into the fixture's deck. Answers 1, having said "not
** loaded", if it was refused: a refusal is what the case is for.
*/
int storage_test_try_load(storage_test_t *t, const bridge_artifact_t *a)
{
    if (!bridge_load(t->b, a))
        return (0);
    printf("not loaded\n");
    return (1);
}

/*
** storage_test_load_reloadable
**
** Loads into a deck a module that is reloadable and may be reloaded. The
** program ends, saying "FAIL: not loaded", if there is no deck or the
** load is refused.
*/
void storage_test_load_reloadable(bridge_t *b, bridge_artifact_t *a)
{
    a->cfg.reloadable = BRIDGE_TRUE;
    a->cfg.hot_reload = BRIDGE_TRUE;
    if (b && !bridge_load(b, a))
        return;
    printf("FAIL: not loaded\n");
    exit(1);
}

/*
** storage_test_reopen
**
** Destroys the fixture's deck, creates another from the fixture's
** deployment with this sink, and loads a module into it. Answers 1, having
** said "FAIL: not loaded", if either is refused.
*/
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
