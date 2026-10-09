/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/support/host_module.c -- a module as a deployment would state it:
** every key said, nothing left at zero.
**
** config.md, *No unaccountable values*
*/

#include "host.h"

/*
** host_run_frames_unsaid
**
** Runs "update" and "present" `count` times, and says nothing.
*/
void host_run_frames_unsaid(bridge_t *b, int count)
{
    int i = 0;

    while (i < count) {
        (void)bridge_run_phase(b, "update");
        (void)bridge_run_phase(b, "present");
        i++;
    }
}

/*
** host_state_plain
**
** States a plain module as a deployment must.
*/
void host_state_plain(bridge_artifact_t *a, const char *identity,
    const char *path)
{
    *a = (bridge_artifact_t){ .backend = "plain", .path = path };
    a->cfg = (bridge_module_config_t){ .identity = identity,
        .abi = BRIDGE_ABI, .trusted = BRIDGE_TRUE, .reloadable = BRIDGE_FALSE,
        .hot_reload = BRIDGE_FALSE, .pinned = BRIDGE_FALSE,
        .on_error = BRIDGE_SKIP, .max_system_failures = 3,
        .work_limit = BRIDGE_NO_LIMIT, .memory_limit = BRIDGE_NO_LIMIT,
        .init_timeout = BRIDGE_NO_LIMIT, .reload_timeout = BRIDGE_NO_LIMIT,
        .shutdown_timeout = BRIDGE_NO_LIMIT, .scratch = 64 << 10,
        .log_ring = 64 * 24, .spans = 16 << 10, .log_level = BRIDGE_INFO,
        .spawns = BRIDGE_NONE, .persistent = BRIDGE_NONE, .changes = 1 << 20,
        .loss_count = BRIDGE_NONE, .config = BRIDGE_NO_CONFIG };
}
