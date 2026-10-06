/* A module whose shutdown never returns: its timeout ends it, and the teardown goes on. */
#include "bridge_wasm.h"

static volatile int going = 1;

BRIDGE_EXPORT(init) int init() { return 0; }

BRIDGE_EXPORT(shutdown) int shutdown()
{
    while (going) {}
    return 0;
}
BRIDGE_EXPORT(reload) int reload() { return 0; }
