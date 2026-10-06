/* Counts up in a component, and can be reloaded. init registers and makes the row; reload only
 * registers, over the row that is already there - docs/seam/host-api/lifecycle.md.
 *
 * Built several ways: STEP is what a pass adds; WIDE changes the struct's body; FAIL_RELOAD
 * makes reload answer 5; NO_EXTRA stops declaring a second component; TRIP fails every pass;
 * ALLOC has reload take forty bytes of the persistent arena, which a share of sixteen lacks. */
#include "bridge_wasm.h"

#ifdef WIDE
BRIDGE_SCHEMA_STRUCT(Tick, { uint32_t n; uint32_t more; });
#else
BRIDGE_SCHEMA_STRUCT(Tick, { uint32_t n; });
#endif
BRIDGE_SCHEMA_STRUCT(Extra, { uint32_t unused; });

static uint32_t tick, ticking;

BRIDGE_EXPORT(advance) int advance()
{
    uint32_t span = bridge_span(ticking, 0, 1);
    uint32_t at = BRIDGE_ROW(Tick, bridge_base(span, 0), 0, bridge_rows(span));
    BRIDGE_SET(Tick, n, at, BRIDGE_GET(Tick, n, at) + STEP);
    BRIDGE_LOG(BRIDGE_LOG_INFO, "tick", BRIDGE_GET(Tick, n, at), STEP);
#ifdef TRIP
    return 7;
#else
    return 0;
#endif
}

/* what init and reload both do: the same components, the same systems */
static void registration()
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};
    tick = BRIDGE_COMPONENT("game:Tick", Tick, BRIDGE_DELETE, BRIDGE_TABLE);
    terms[0].slot = tick;
    ticking = BRIDGE_SYSTEM("advance", "update", terms, 1, nullptr, 0);
}

BRIDGE_EXPORT(init) int init()
{
    Tick t = { 100 };
    Extra e = {};
    uint32_t extra;
    registration();
    BRIDGE_ATTACH(bridge_spawn(), tick, t);
    extra = BRIDGE_COMPONENT("game:Extra", Extra, BRIDGE_DELETE, BRIDGE_TABLE);
    BRIDGE_ATTACH(bridge_spawn(), extra, e);
    return 0;
}

BRIDGE_EXPORT(reload) int reload()
{
    registration();
#ifndef NO_EXTRA
    BRIDGE_COMPONENT("game:Extra", Extra, BRIDGE_DELETE, BRIDGE_TABLE);
#endif
#ifdef ALLOC
    bridge_alloc(40);
#endif
#ifdef FAIL_RELOAD
    return 5;
#else
    return 0;
#endif
}

BRIDGE_EXPORT(shutdown) int shutdown()
{
    BRIDGE_LOG(BRIDGE_LOG_INFO, "shutdown", STEP, 0);
    return 0;
}
