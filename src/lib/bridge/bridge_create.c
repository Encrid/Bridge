/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <stdlib.h>

#include "bridge_inside.h"
#include "say.h"

static const char NO_WORLD[] = "out of memory: there was none for the world";

static bridge_world_t *make_bridge(bridge_sink_fn_t sink, void *ud)
{
    bridge_world_t *w = calloc(1, sizeof(*w));
    bridge_record_t r = { .text = NO_WORLD, .level = BRIDGE_ERR };

    if (!w) {
        r.len = sizeof(NO_WORLD) - 1;
        sink(ud, &r);
        return (nullptr);
    }
    w->granule = 1;
    w->sink = sink;
    w->ud = ud;
    return (w);
}

static int check_config(bridge_world_t *w, const bridge_config_t *cfg)
{
    uint32_t backends = cfg->backend_count ? cfg->backend_count : 1;
    uint32_t phases = cfg->phase_count ? cfg->phase_count : 1;

    if (!cfg->abi) {
        say_error(w, "abi is unset");
        return (1);
    }
    if (cfg->abi != BRIDGE_HOST_ABI) {
        say_error(w, "the host is of host ABI %u, and this is %u", cfg->abi,
            (unsigned)BRIDGE_HOST_ABI);
        return (1);
    }
    w->backends = calloc(backends, sizeof(*w->backends));
    w->phases = calloc(phases, sizeof(*w->phases));
    return (bridge_lacks(w, w->backends && w->phases ? w : nullptr,
        "its backends and its phases"));
}

static int start_pool(bridge_world_t *w, const bridge_config_t *cfg)
{
    if (!cfg->threads) {
        say_error(w, "threads is unset");
        return (1);
    }
    w->pool = pool_create(cfg->threads - 1, cfg->budget.systems);
    return (bridge_lacks(w, w->pool, "the threads that run a phase"));
}

static int build_bridge(bridge_world_t *w, const bridge_config_t *cfg)
{
    if (check_config(w, cfg) || bridge_open_backends(w, cfg))
        return (1);
    if (bridge_name_phases(w, cfg))
        return (1);
    if (schedule_order(w, cfg->order, cfg->order_count))
        return (1);
    if (start_pool(w, cfg) || block_create(w, &cfg->budget))
        return (1);
    return (schedule_room(w));
}

bridge_world_t *bridge_create(const bridge_config_t *cfg, bridge_sink_fn_t sink,
    void *ud)
{
    bridge_world_t *w = sink && cfg ? make_bridge(sink, ud) : nullptr;

    if (!w)
        return (nullptr);
    if (!build_bridge(w, cfg))
        return (w);
    (void)bridge_destroy(w);
    return (nullptr);
}
