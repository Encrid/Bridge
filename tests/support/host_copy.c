/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "host_copy.h"

static const char *const *names(host_copy_t *c, const char *const *list,
    int64_t count)
{
    const char **to = nullptr;

    if (!list || count <= 0)
        return (list);
    to = host_take(c, list, sizeof(*list) * (size_t)count);
    for (int64_t i = 0; i < count; i++)
        to[i] = host_take_text(c, list[i]);
    return (to);
}

static const bridge_setting_t *settings(host_copy_t *c,
    const bridge_setting_t *list, uint32_t count)
{
    bridge_setting_t *to = nullptr;

    if (!list || !count)
        return (list);
    to = host_take(c, list, sizeof(*list) * count);
    for (uint32_t i = 0; i < count; i++) {
        to[i].key = host_take_text(c, list[i].key);
        if (list[i].kind == BRIDGE_STRING)
            to[i].v.s = host_take_text(c, list[i].v.s);
    }
    return (to);
}

static const bridge_phase_name_t *phases(host_copy_t *c,
    const bridge_phase_name_t *list, uint32_t count)
{
    bridge_phase_name_t *to = nullptr;

    if (!list || !count)
        return (list);
    to = host_take(c, list, sizeof(*list) * count);
    for (uint32_t i = 0; i < count; i++)
        to[i] = (bridge_phase_name_t){ host_take_text(c, list[i].from),
            host_take_text(c, list[i].to) };
    return (to);
}

const bridge_artifact_t *host_copy(host_copy_t *c, const bridge_artifact_t *a,
    uint32_t count)
{
    bridge_artifact_t *to = nullptr;
    bridge_module_config_t *cfg = nullptr;

    *c = (host_copy_t){ malloc(HOST_COPY_ROOM), sizeof(void *),
        HOST_COPY_ROOM };
    to = host_take(c, a, sizeof(*a) * count);
    for (uint32_t i = 0; i < count; i++) {
        cfg = &to[i].cfg;
        cfg->identity = host_take_text(c, cfg->identity);
        cfg->config = host_take_text(c, cfg->config);
        cfg->read = names(c, cfg->read, cfg->read_count);
        cfg->write = names(c, cfg->write, cfg->write_count);
        cfg->losses = names(c, cfg->losses, cfg->loss_count);
        cfg->settings = settings(c, cfg->settings, cfg->setting_count);
        cfg->phases = phases(c, cfg->phases, cfg->phase_count);
    }
    return (to);
}
