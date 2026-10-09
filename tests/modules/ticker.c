/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/ticker.c -- counts up in a component, and can be
** reloaded. init registers and makes the row; reload only registers, over
** the row that is already there.
**
** Built several ways: STEP is what a pass adds; WIDE changes the struct's
** body; FAIL_RELOAD makes reload answer 5; NO_EXTRA stops declaring a
** second component; TRIP fails every pass; ALLOC has reload take forty
** bytes of the persistent arena, which a share of sixteen lacks.
**
** lifecycle.md, *The runtime lifecycle*
*/

#include "test_offsets.h"
#include "ticker.h"

static uint32_t tick;
static uint32_t ticking;

/*
** advance
**
** Adds STEP to the count of the first row, and says the count and STEP.
** The build with TRIP then answers 7.
*/
BRIDGE_EXPORT(advance) int advance(void)
{
    uint32_t span = bridge_get_span(ticking, 0, 1);
    uint32_t at = BRIDGE_ROW(tick_t, bridge_get_base(span, 0), 0,
        bridge_count_rows(span));

    BRIDGE_SET(tick_t, n, at, BRIDGE_GET(tick_t, n, at) + STEP);
    BRIDGE_LOG(BRIDGE_LOG_INFO, "tick", BRIDGE_GET(tick_t, n, at), STEP);
#ifdef TRIP
    return (7);
#else
    return (0);
#endif
}

/*
** register_tick
**
** Does what init and reload both do: the same component, the same system.
*/
static void register_tick(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};

    tick = BRIDGE_COMPONENT("game:Tick", tick_t, BRIDGE_DELETE, BRIDGE_TABLE);
    terms[0].slot = tick;
    ticking = BRIDGE_ADD_SYSTEM("advance", "update", terms, 1, nullptr, 0);
}

/*
** init
**
** Registers, spawns the entity that counts, from 100, and declares a
** second component with an entity of its own.
*/
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

/*
** reload
**
** Registers, and declares the second component again unless the build is
** NO_EXTRA. The build with ALLOC takes forty bytes of the persistent arena,
** and the build with FAIL_RELOAD answers 5.
*/
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

/*
** shutdown
**
** Says that it was shut down, and its STEP.
*/
BRIDGE_EXPORT(shutdown) int shutdown(void)
{
    BRIDGE_LOG(BRIDGE_LOG_INFO, "shutdown", STEP, 0);
    return (0);
}
