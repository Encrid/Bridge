/* A system that fails FAILS passes out of every OF. Built three ways: always, one in three,
 * two in three - docs/bridge/run/failure.md, *The budget*. */
#include "bridge-wasm.h"

static uint32_t passes;

BRIDGE_EXPORT(trip) int trip(void)
{
    uint32_t n = passes ++ % OF;
    BRIDGE_LOG(BRIDGE_LOG_INFO, "trip", passes, n < FAILS);
    return n < FAILS ? 7 : 0;
}

BRIDGE_EXPORT(init) int init(void)
{
    int none = 0;
    BRIDGE_SYSTEM("trip", "update", &none, 0);
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown(void) { return 0; }
