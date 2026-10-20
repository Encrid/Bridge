/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "module.h"

static int call_first(module_t *m, const module_start_t *st,
    backend_err_t *err)
{
    const bridge_module_config_t *cfg = &st->a->cfg;
    const backend_api_t *api = &m->on->api;
    module_call_t call = { nullptr, st->entry, cfg->init_timeout };
    int failed;

    if (!strcmp(st->entry, "reload"))
        call.timeout = cfg->reload_timeout;
    call.f = api->instance_get_func(m->instance, st->entry, err);
    if (!call.f)
        return (1);
    m->in_init = 1;
    failed = module_call(m, &call, err);
    m->in_init = 0;
    return (failed);
}

static int check_losses(const module_t *m, backend_err_t *err)
{
    uint32_t i = 0;

    while (i < m->loss_count && m->loss_rows[i])
        i++;
    if (i == m->loss_count)
        return (0);
    return (module_fail(err, "it accepts the loss of '%.100s', and loses "
        "nothing of that name", m->losses[i]));
}

static int find_exports(module_t *m, backend_err_t *err)
{
    const backend_api_t *api = &m->on->api;
    bridge_system_t *s;
    uint32_t i = 0;

    while (i < m->system_count + m->observer_count) {
        s = i < m->system_count ? m->systems[i]
            : m->observers[i - m->system_count];
        s->fn = api->instance_get_func(m->instance, s->name, err);
        if (!s->fn)
            return (1);
        i++;
    }
    m->shutdown = api->instance_get_func(m->instance, "shutdown", err);
    if (!m->shutdown)
        return (1);
    return (!api->instance_get_func(m->instance, "reload", err));
}

static int check_spans(const module_t *m, backend_err_t *err)
{
    uint32_t tables = storage_count_tables(m->b->storage);
    uint64_t need = sizeof(bridge_query_t) * m->system_count
        + sizeof(bridge_list_t) * m->observer_count
        + 4ull * m->declared_count * tables
        + 8ull * tables
        + 16ull * m->declared_count
        + 4ull + 8ull * BRIDGE_OWNED_CAP;

    need = (need + 7u) & ~7ull;
    if (need <= m->spans_len)
        return (0);
    return (module_fail(err, "its spans of %u bytes do not hold a "
        "column_base of %u components by %u tables, which with its queries "
        "is %llu bytes", m->spans_len, m->declared_count, tables,
        (unsigned long long)need));
}

int module_start(module_t *m, const module_start_t *st, backend_err_t *err)
{
    bool loading = strcmp(st->entry, "reload") != 0;

    if (module_make_instance(m, st, err) || call_first(m, st, err))
        return (1);
    if (check_losses(m, err))
        return (1);
    if (loading && config_seed(m, &st->a->cfg, err, false))
        return (1);
    return (find_exports(m, err) || check_spans(m, err));
}
