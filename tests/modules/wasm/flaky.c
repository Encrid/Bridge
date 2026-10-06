/* A system that fails FAILS passes out of every OF. Built four ways: always, one in three,
 * two in three, and always beside a system of the next phase that never fails -
 * docs/bridge/run/failure.md, *The budget*. */
#include "bridge_wasm.h"

static uint32_t passes;

BRIDGE_EXPORT(trip) int trip()
{
    uint32_t n = passes ++ % OF;
    BRIDGE_LOG(BRIDGE_LOG_INFO, "trip", passes, n < FAILS);
    return n < FAILS ? 7 : 0;
}

#ifdef CALM
BRIDGE_EXPORT(calm) int calm() { return 0; }
#endif

BRIDGE_EXPORT(init) int init()
{
    int none = 0;
    BRIDGE_ADD_SYSTEM("trip", "update", &none, 0, nullptr, 0);
#ifdef CALM
    BRIDGE_ADD_SYSTEM("calm", "present", &none, 0, nullptr, 0);
#endif
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown() { return 0; }
BRIDGE_EXPORT(reload) int reload() { return 0; }
