/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "storage_test.h"

static const char *const GROUP_PHASES[2] = { "update", "present" };

static bridge_artifact_t share(const char *identity, const char *path)
{
    bridge_artifact_t a = { .backend = "guarded", .bytes = path,
        .len = strlen(path) };

    a.cfg = (bridge_module_config_t){ .identity = identity, .scratch = 64 << 10,
        .abi = BRIDGE_ABI, .reloadable = BRIDGE_TRUE, .hot_reload = BRIDGE_TRUE,
        .pinned = BRIDGE_FALSE, .on_error = BRIDGE_SKIP,
        .max_system_failures = 1, .work_limit = BRIDGE_NO_LIMIT,
        .memory_limit = BRIDGE_NO_LIMIT, .element_limit = BRIDGE_NO_LIMIT,
        .init_timeout = BRIDGE_NO_LIMIT,
        .reload_timeout = BRIDGE_NO_LIMIT, .shutdown_timeout = BRIDGE_NO_LIMIT,
        .log_ring = 256 * 32, .spans = 1 << 20, .log_level = BRIDGE_INFO,
        .spawns = 4, .persistent = 64, .changes = 1 << 20,
        .loss_count = BRIDGE_NONE, .config = BRIDGE_NO_CONFIG };
    return (a);
}

enum storage_test_end storage_test_group_live(storage_test_t *t)
{
    bridge_artifact_t a[2];

    t->cfg.phases = GROUP_PHASES;
    t->cfg.phase_count = 2;
    t->w = bridge_create(&t->cfg, storage_test_hear_memory, t);
    if (!t->w)
        return (STORAGE_TEST_DONE);
    a[0] = share("game:one@0.1.0", BUILT_DIR "/share1" BUILT_SUFFIX);
    a[1] = share("game:two@0.1.0", BUILT_DIR "/share1" BUILT_SUFFIX);
    if (!bridge_load(t->w, &a[0]) && !bridge_load(t->w, &a[1])) {
        a[0] = share("game:one@0.2.0", BUILT_DIR "/share2" BUILT_SUFFIX);
        a[1] = share("game:two@0.2.0", BUILT_DIR "/share2" BUILT_SUFFIX);
        a[0].cfg.persistent = 128;
        a[1].cfg.persistent = 128;
        (void)bridge_reload(t->w, a, 2);
    }
    return (STORAGE_TEST_DONE);
}
