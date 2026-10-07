/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** src/bridge_create.c -- a deck made of a deployment: its backends opened,
** its phases named, its threads started and its storage built.
*/

#include <stdlib.h>

#include "bridge_inside.h"
#include "say.h"

static const char NO_DECK[] = "out of memory: there was none for the deck";

/*
** make_bridge
**
** Answers an empty deck that knows its sink. With no memory for it there
** is no deck to say so through: the record is made here.
*/
static bridge_t *make_bridge(bridge_sink_fn_t sink, void *ud)
{
    bridge_t *b = calloc(1, sizeof(*b));
    bridge_record_t r = { .text = NO_DECK, .level = BRIDGE_ERR };

    if (!b) {
        r.len = sizeof(NO_DECK) - 1;
        sink(ud, &r);
        return (nullptr);
    }
    b->granule = 1;
    b->sink = sink;
    b->ud = ud;
    return (b);
}

/*
** check_config
**
** Reads the host seam's number before anything else of the struct is, and
** makes room for the backends and the phases it then counts.
**
** host_api.md, *A seam has a number*
*/
static int check_config(bridge_t *b, const bridge_config_t *cfg)
{
    uint32_t backends = cfg->backend_count ? cfg->backend_count : 1;
    uint32_t phases = cfg->phase_count ? cfg->phase_count : 1;

    if (!cfg->abi) {
        say_error(b, "abi is unset");
        return (1);
    }
    if (cfg->abi != BRIDGE_HOST_ABI) {
        say_error(b, "the host is of host ABI %u, and this is %u", cfg->abi,
            (unsigned)BRIDGE_HOST_ABI);
        return (1);
    }
    b->backends = calloc(backends, sizeof(*b->backends));
    b->phases = calloc(phases, sizeof(*b->phases));
    return (bridge_lacks(b, b->backends && b->phases ? b : nullptr,
        "its backends and its phases"));
}

/*
** start_pool
**
** Starts the threads that run a phase. How many is not ours to guess, and a
** job is a module's systems of one level, so there are never more jobs than
** systems.
**
** config.md, *No unaccountable values*
*/
static int start_pool(bridge_t *b, const bridge_config_t *cfg)
{
    if (!cfg->threads) {
        say_error(b, "threads is unset");
        return (1);
    }
    b->pool = pool_create(cfg->threads - 1, cfg->budget.systems);
    return (bridge_lacks(b, b->pool, "the threads that run a phase"));
}

/*
** build_bridge
**
** Makes of an empty deck what the deployment states, a part at a time.
** 1 at the first part that is refused, with a record.
*/
static int build_bridge(bridge_t *b, const bridge_config_t *cfg)
{
    if (check_config(b, cfg) || bridge_open_backends(b, cfg))
        return (1);
    if (bridge_name_phases(b, cfg))
        return (1);
    if (schedule_order(b, cfg->order, cfg->order_count))
        return (1);
    if (start_pool(b, cfg) || block_create(b, &cfg->budget))
        return (1);
    return (schedule_room(b));
}

/*
** bridge_create
**
** Makes a deck of a deployment. nullptr if the deployment cannot start;
** which reason, the sink was told. A null sink or deployment is the one
** failure no record can explain, and a host that passed null already knows.
*/
bridge_t *bridge_create(const bridge_config_t *cfg, bridge_sink_fn_t sink,
    void *ud)
{
    bridge_t *b = sink && cfg ? make_bridge(sink, ud) : nullptr;

    if (!b)
        return (nullptr);
    if (!build_bridge(b, cfg))
        return (b);
    (void)bridge_destroy(b);
    return (nullptr);
}
