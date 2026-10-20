/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "ticker.h"

static uint32_t tick;
static uint32_t ticking;

BRIDGE_EXPORT(advance) int advance(void)
{
    uint32_t span = bridge_get_span(ticking, 0);
    uint32_t table = bridge_span_table(span);

    BRIDGE_SET(tick_t, n, tick, table, 0,
        BRIDGE_GET(tick_t, n, tick, table, 0) + STEP);
    BRIDGE_LOG(BRIDGE_LOG_INFO, "tick", BRIDGE_GET(tick_t, n, tick, table, 0),
        STEP);
#ifdef TRIP
    return (7);
#else
    return (0);
#endif
}

static void register_tick(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};

    tick = BRIDGE_COMPONENT("game:Tick", tick_t, BRIDGE_DELETE, BRIDGE_TABLE);
    terms[0].slot = tick;
    ticking = BRIDGE_ADD_SYSTEM("advance", "update", terms, 1, nullptr, 0, nullptr, 0);
}

BRIDGE_EXPORT(init) int init(void)
{
    tick_t t = { 100 };
    extra_t e = {};
    uint32_t extra;

    register_tick();
    BRIDGE_ATTACH(bridge_spawn(), tick, t);
    extra = BRIDGE_COMPONENT("game:Extra", extra_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    BRIDGE_ATTACH(bridge_spawn(), extra, e);
    return (0);
}

BRIDGE_EXPORT(reload) int reload(void)
{
    register_tick();
#ifndef NO_EXTRA
    BRIDGE_COMPONENT("game:Extra", extra_t, BRIDGE_DELETE, BRIDGE_TABLE);
#endif
#ifdef ALLOC
    bridge_alloc(40);
#endif
#ifdef FAIL_RELOAD
    return (5);
#else
    return (0);
#endif
}

BRIDGE_EXPORT(shutdown) int shutdown(void)
{
    BRIDGE_LOG(BRIDGE_LOG_INFO, "shutdown", STEP, 0);
    return (0);
}
