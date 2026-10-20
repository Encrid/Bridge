/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <stdlib.h>

#include "host.h"

void host_run_frames_unsaid(bridge_world_t *w, int count)
{
    int i = 0;

    while (i < count) {
        (void)bridge_run_phase(w, "update");
        (void)bridge_run_phase(w, "present");
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

static void state_keys(bridge_artifact_t *a, const char *identity)
{
    *a = (bridge_artifact_t){ .backend = "wasm" };
    a->cfg = (bridge_module_config_t){ .identity = identity,
        .abi = BRIDGE_ABI, .reloadable = BRIDGE_FALSE,
        .hot_reload = BRIDGE_FALSE, .pinned = BRIDGE_FALSE,
        .on_error = BRIDGE_SKIP, .max_system_failures = 3,
        .work_limit = BRIDGE_NO_LIMIT, .memory_limit = BRIDGE_NO_LIMIT,
        .init_timeout = BRIDGE_NO_LIMIT, .reload_timeout = BRIDGE_NO_LIMIT,
        .shutdown_timeout = BRIDGE_NO_LIMIT, .scratch = 64 << 10,
        .log_ring = 64 * 24, .spans = 16 << 10, .log_level = BRIDGE_INFO,
        .spawns = BRIDGE_NONE, .persistent = BRIDGE_NONE, .changes = 1 << 20,
        .loss_count = BRIDGE_NONE, .config = BRIDGE_NO_CONFIG };
}

void host_state_wasm(bridge_artifact_t *a, const char *identity,
    const char *file)
{
    state_keys(a, identity);
    a->bytes = host_read_module(file, &a->len);
    a->cfg.work_limit = 10'000'000;
    a->cfg.memory_limit = 16 << 20;
    a->cfg.init_timeout = 2000 * HOST_MS;
    a->cfg.reload_timeout = 2000 * HOST_MS;
    a->cfg.shutdown_timeout = 2000 * HOST_MS;
}
