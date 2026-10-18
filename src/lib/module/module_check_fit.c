/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdckdint.h>

#include "module.h"

int module_lay_private(const bridge_module_config_t *cfg, uint64_t granule,
    module_private_t *at)
{
    uint32_t end;

    if (ckd_add(&end, MODULE_SCRATCH_AT, cfg->scratch)
        || ckd_add(&end, end, 7u))
        return (1);
    at->ring = end & ~7u;
    if (ckd_add(&end, at->ring, cfg->log_ring) || ckd_add(&end, end, 7u))
        return (1);
    at->spans = end & ~7u;
    if (ckd_add(&at->end, at->spans, cfg->spans))
        return (1);
    return ((uint64_t)at->end + granule - 1 > UINT32_MAX);
}

static int check_log_level(const module_check_t *k)
{
    uint32_t level = k->cfg->log_level;

    if (module_check_set(k, "log_level", level))
        return (1);
    if (level <= BRIDGE_LEVEL_MOST)
        return (0);
    return (module_refuse(k->b, k->id, "its log_level of %u is not one it "
        "can be, which is 1 to %u", level, BRIDGE_LEVEL_MOST));
}

int module_check_fit(const module_check_t *k)
{
    const bridge_module_config_t *cfg = k->cfg;
    constexpr uint32_t record = sizeof(bridge_log_record_t);
    module_private_t at;

    if (check_log_level(k))
        return (1);
    if (!cfg->config || !*cfg->config)
        return (module_refuse(k->b, k->id, "config is unset"));
    if (cfg->spawns > (int64_t)k->b->entities)
        return (module_refuse(k->b, k->id, "its spawns of %lld is more than "
            "[budget] entities of %u", (long long)cfg->spawns, k->b->entities));
    if (module_lay_private(cfg, k->on->granule, &at))
        return (module_refuse(k->b, k->id, "its scratch of %u, log_ring of %u "
            "and spans of %u bytes come to more than an offset of 32 bits can "
            "reach", cfg->scratch, cfg->log_ring, cfg->spans));
    if (cfg->log_ring >= record)
        return (0);
    return (module_refuse(k->b, k->id, "a log_ring of %u bytes holds no "
        "record, and one is %u", cfg->log_ring, record));
}
