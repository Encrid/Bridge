/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>
#include <string.h>

#include "module.h"

static char **copy_list(const char *const *list, uint32_t count)
{
    char **out = calloc(count ? count : 1, sizeof(*out));
    bool whole = out != nullptr;
    uint32_t i = 0;

    while (whole && i < count) {
        out[i] = copy_text(list[i], strlen(list[i]));
        whole = out[i] != nullptr;
        i++;
    }
    if (whole)
        return (out);
    while (out && i) {
        i--;
        free(out[i]);
    }
    free(out);
    return (nullptr);
}

static void copy_stated(module_t *m, const bridge_module_config_t *cfg)
{
    m->pinned = cfg->pinned == BRIDGE_TRUE;
    m->thread = os_get_thread_id();
    m->on_error = cfg->on_error;
    m->hot_reload = cfg->hot_reload == BRIDGE_TRUE;
    m->max_failures = cfg->max_system_failures;
    m->shutdown_timeout = cfg->shutdown_timeout;
    m->work_limit = cfg->work_limit;
    m->spawns = cfg->spawns;
    m->persistent = cfg->persistent;
    m->losses = cfg->losses;
    m->loss_count = cfg->loss_count > 0 ? (uint32_t)cfg->loss_count : 0;
}

static bool alloc_room(module_t *m, const bridge_module_config_t *cfg)
{
    const char *id = cfg->identity;
    const char *colon = strchr(id, ':');
    size_t losses = m->loss_count ? m->loss_count : 1;

    m->identity = copy_text(id, strlen(id));
    m->stem = copy_text(id, bridge_get_stem_length(id));
    m->ns = copy_text(id, (size_t)(colon - id));
    m->loss_rows = calloc(losses, sizeof(*m->loss_rows));
    if (cfg->spawns > 0)
        m->ids = calloc((size_t)cfg->spawns, sizeof(*m->ids));
    if (m->ids)
        m->id_room = (uint32_t)cfg->spawns;
    if (cfg->changes > 0)
        m->queue = malloc((size_t)cfg->changes);
    if (m->queue)
        m->queue_room = (uint32_t)cfg->changes;
    if (!m->identity || !m->stem || !m->ns || !m->loss_rows)
        return (false);
    return ((cfg->spawns <= 0 || m->ids) && (cfg->changes <= 0 || m->queue));
}

static bool copy_phases(const bridge_t *b, module_t *m,
    const bridge_module_config_t *cfg)
{
    size_t count = cfg->phase_count ? cfg->phase_count : 1;
    const bridge_phase_name_t *n = cfg->phases;
    bool whole;
    uint32_t i = 0;

    m->phase_from = calloc(count, sizeof(*m->phase_from));
    m->phase_to = calloc(count, sizeof(*m->phase_to));
    whole = m->phase_from && m->phase_to;
    while (whole && i < cfg->phase_count) {
        m->phase_to[i] = bridge_find_phase(b->phases, b->phase_count, n[i].to,
            strlen(n[i].to));
        m->phase_from[i] = copy_text(n[i].from, strlen(n[i].from));
        whole = m->phase_from[i] != nullptr;
        i++;
    }
    m->phase_names = m->phase_from ? i : 0;
    return (whole);
}

module_t *module_record(bridge_t *b, const bridge_artifact_t *a,
    open_backend_t *o)
{
    const bridge_module_config_t *cfg = &a->cfg;
    module_t *m = calloc(1, sizeof(*m));
    bool whole;

    if (bridge_lacks(b, m, "a module's record"))
        return (nullptr);
    m->b = b;
    m->on = o;
    copy_stated(m, cfg);
    whole = alloc_room(m, cfg);
    whole = copy_phases(b, m, cfg) && whole;
    m->read = copy_list(cfg->read, cfg->read_count);
    m->read_count = m->read ? cfg->read_count : 0;
    m->write = copy_list(cfg->write, cfg->write_count);
    m->write_count = m->write ? cfg->write_count : 0;
    if (whole && m->read && m->write)
        return (m);
    (void)bridge_lacks(b, nullptr, "a module's record");
    module_discard(b, m);
    return (nullptr);
}
