/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>
#include <string.h>

#include "host.h"

void host_state_built(bridge_artifact_t *a, const char *identity,
    const char *path)
{
    host_state_plain(a, identity, path);
}

static const char *keep_path(const char *file)
{
    static char paths[32][512];
    static uint32_t next;
    char *path = paths[next++ % 32];

    snprintf(path, sizeof(paths[0]), "%s/%s%s", BUILT_DIR, file, BUILT_SUFFIX);
    return (path);
}

void host_name_module(bridge_artifact_t *a, const char *file)
{
    a->path = nullptr;
    a->bytes = keep_path(file);
    a->len = strlen(a->bytes);
}

void host_state_guarded(bridge_artifact_t *a, const char *identity,
    const char *file)
{
    host_state_plain(a, identity, nullptr);
    host_name_module(a, file);
    a->backend = "guarded";
    a->cfg.work_limit = 10'000'000;
    a->cfg.memory_limit = 16 << 20;
    a->cfg.element_limit = 1 << 16;
    a->cfg.init_timeout = 2000 * HOST_MS;
    a->cfg.reload_timeout = 2000 * HOST_MS;
    a->cfg.shutdown_timeout = 2000 * HOST_MS;
}
