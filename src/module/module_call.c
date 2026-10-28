/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdio.h>

#include "module.h"

static void place_regions(module_t *m, const bridge_module_config_t *cfg,
    const module_private_t *at)
{
    bridge_header_t *header = (bridge_header_t *)m->priv;

    m->scratch = MODULE_SCRATCH_AT;
    m->scratch_len = cfg->scratch;
    m->ring = at->ring;
    m->ring_len = cfg->log_ring / (uint32_t)sizeof(bridge_log_record_t);
    m->spans = at->spans;
    m->spans_len = cfg->spans;
    header->log_level = cfg->log_level;
    header->system = UINT32_MAX;
}

int module_take_private(module_t *m, const bridge_module_config_t *cfg,
    backend_err_t *err)
{
    open_backend_t *o = m->on;
    module_private_t at;
    uint64_t len;
    uint64_t reserve;

    (void)module_lay_private(cfg, o->granule, &at);
    len = ((uint64_t)at.end + o->granule - 1) / o->granule * o->granule;
    reserve = (len > o->reservation ? len : o->reservation) + o->guard;
    m->priv = os_reserve((size_t)reserve, (size_t)len);
    if (!m->priv)
        return (module_fail(err, "the operating system would not give its "
            "private memory of %llu bytes", (unsigned long long)len));
    m->priv_len = (size_t)len;
    m->priv_reserve = (size_t)reserve;
    place_regions(m, cfg, &at);
    m->memory = o->api.memory_adopt(o->be, m->priv, len, err);
    return (!m->memory);
}

static int say_trap(const char *name, backend_err_t *err)
{
    char reason[sizeof(err->text)];

    snprintf(reason, sizeof(reason), "%s", err->text);
    return (module_fail(err, "its %s %.200s", name, reason));
}

int module_call(module_t *m, const module_call_t *call, backend_err_t *err)
{
    const backend_api_t *api = &m->on->api;
    int32_t status = 0;
    int trapped;

    if (call->timeout > 0)
        api->instance_set_timeout(m->instance, (uint64_t)call->timeout);
    m->refused = 0;
    m->turn = TURN_LIFECYCLE | m->w->turns++;
    if (!abi_marks(m, 0))
        return (module_fail(err, "out of memory: there was none to call its "
            "%s with", call->name));
    trapped = api->call(call->f, &status, err);
    if (m->refused)
        return (module_fail(err, "%s", m->why));
    if (trapped)
        return (say_trap(call->name, err));
    if (status)
        return (module_fail(err, "its %s answered %d", call->name, status));
    return (0);
}
