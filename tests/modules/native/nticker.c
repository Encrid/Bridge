/* ticker.c as a shared library. A process cannot hold two copies of one library, so a reload
 * of this one shuts the old instance down before the new one is opened. */
#include "bridge-native.h"

BRIDGE_SCHEMA_STRUCT(Tick, { uint32_t n; });

static const bridge_api *api;
static uint32_t tick, ticking;

BRIDGE_NATIVE_BIND(api)

BRIDGE_NATIVE(advance)
{
    const bridge_query *q = bridge_native_query(api, ticking);
    auto t = (Tick *)(api->shared_base + bridge_native_span(api, q, 0, 1)[2]);
    t->n += STEP;
    bridge_native_log(api, BRIDGE_LOG_INFO, "tick", 4, t->n, STEP);
    return 0;
}

static void registration()
{
    bridge_term terms[1] = {{ 0, BRIDGE_INOUT, BRIDGE_AND, 0 }};
    tick = BRIDGE_NATIVE_COMPONENT(api, "game:Tick", Tick, BRIDGE_DELETE);
    terms[0].slot = tick;
    ticking = BRIDGE_NATIVE_SYSTEM(api, "advance", "update", terms, 1);
}

BRIDGE_NATIVE(init)
{
    Tick t = { 100 };
    registration();
    api->attach(api->spawn(), tick, bridge_native_put(api, &t, sizeof t), sizeof t);
    return 0;
}

BRIDGE_NATIVE(reload)
{
    registration();
#ifdef FAIL_RELOAD
    return 5;
#else
    return 0;
#endif
}

BRIDGE_NATIVE(shutdown)
{
    bridge_native_log(api, BRIDGE_LOG_INFO, "shutdown", 8, STEP, 0);
    return 0;
}
