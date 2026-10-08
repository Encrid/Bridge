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

#include <stdio.h>
#include <stdlib.h>

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

static void *read_whole(FILE *f, size_t *len)
{
    void *data;

    fseek(f, 0, SEEK_END);
    *len = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);
    data = malloc(*len);
    if (data && fread(data, 1, *len, f) != *len) {
        free(data);
        data = nullptr;
    }
    fclose(f);
    return (data);
}

/*
** host_read_module
**
** Answers the bytes of the wasm module of a name, and how many.
*/
void *host_read_module(const char *file, size_t *len)
{
    char path[1024];
    FILE *f;
    void *data = nullptr;

    snprintf(path, sizeof(path), "%s/%s.wasm", MODULE_DIR, file);
    f = fopen(path, "rb");
    if (f)
        data = read_whole(f, len);
    if (data)
        return (data);
    printf("cannot read %s\n", path);
    exit(2);
}

/*
** host_state_native
**
** States a native module as a deployment must.
*/
void host_state_native(bridge_artifact_t *a, const char *identity,
    const char *path)
{
    *a = (bridge_artifact_t){ .backend = "native", .path = path };
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

/*
** host_state_wasm
**
** States a wasm module of tests/modules, with the limits that backend can
** keep.
*/
void host_state_wasm(bridge_artifact_t *a, const char *identity,
    const char *file)
{
    host_state_native(a, identity, nullptr);
    a->bytes = host_read_module(file, &a->len);
    a->backend = "wasm";
    a->cfg.trusted = BRIDGE_FALSE;
    a->cfg.work_limit = 10'000'000;
    a->cfg.memory_limit = 16 << 20;
    a->cfg.init_timeout = 2000 * HOST_MS;
    a->cfg.reload_timeout = 2000 * HOST_MS;
    a->cfg.shutdown_timeout = 2000 * HOST_MS;
}
