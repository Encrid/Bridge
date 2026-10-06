/* ticker.c as a shared library. A process cannot hold two copies of one library, so a reload
 * of this one shuts the old instance down before the new one is opened. */
#include "bridge_native.h"

#define TICK_FIELDS { uint32_t n; }
BRIDGE_SCHEMA_STRUCT_TYPE(Tick, TICK_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(Tick, TICK_FIELDS);

static uint32_t tick, ticking;


BRIDGE_NATIVE(advance)
{
    const bridge_query_t *q = bridge_native_get_query(api, ticking);
    auto t = (Tick *)(api->shared_base + bridge_native_get_span(api, q, 0, 1)[2]);
    t->n += STEP;
    bridge_native_log(api, BRIDGE_LOG_INFO, &(bridge_line_t){ "tick", 4, t->n, STEP });
    return 0;
}

static void registration(const bridge_api_t *api)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};
    tick = BRIDGE_NATIVE_COMPONENT(api, "game:Tick", Tick, BRIDGE_DELETE, BRIDGE_TABLE);
    terms[0].slot = tick;
    ticking = BRIDGE_NATIVE_ADD_SYSTEM(api, "advance", "update", terms, 1, nullptr, 0);
}

BRIDGE_NATIVE(init)
{
    Tick t = { 100 };
    registration(api);
    BRIDGE_NATIVE_ATTACH(api, api->spawn(api), tick, t);
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
    bridge_native_log(api, BRIDGE_LOG_INFO, &(bridge_line_t){ "shutdown", 8, STEP, 0 });
    return 0;
}
