/* ticker.c as a shared library. A process cannot hold two copies of one library, so a reload
 * of this one shuts the old instance down before the new one is opened. */
#include "bridge-native.h"

BRIDGE_SCHEMA_STRUCT(Tick, { uint32_t n; });

static uint32_t tick, ticking;


BRIDGE_NATIVE(advance)
{
    const bridge_query *q = bridge_native_query(api, ticking);
    auto t = (Tick *)(api->shared_base + bridge_native_span(api, q, 0, 1)[2]);
    t->n += STEP;
    bridge_native_log(api, BRIDGE_LOG_INFO, "tick", 4, t->n, STEP);
    return 0;
}

static void registration(const bridge_api *api)
{
    bridge_term terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};
    tick = BRIDGE_NATIVE_COMPONENT(api, "game:Tick", Tick, BRIDGE_DELETE);
    terms[0].slot = tick;
    ticking = BRIDGE_NATIVE_SYSTEM(api, "advance", "update", terms, 1);
}

BRIDGE_NATIVE(init)
{
    Tick t = { 100 };
    registration(api);
    api->attach(api, api->spawn(api), tick, bridge_native_put(api, &t, sizeof t), sizeof t);
    return 0;
}

BRIDGE_NATIVE(reload)
{
    registration(api);
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
