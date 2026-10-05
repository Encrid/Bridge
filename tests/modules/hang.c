/* A module whose shutdown never returns: its timeout ends it, and the teardown goes on. */
#include "bridge-wasm.h"

static volatile int going = 1;

BRIDGE_EXPORT(init) int init(void) { return 0; }

BRIDGE_EXPORT(shutdown) int shutdown(void)
{
    while (going) {}
    return 0;
}
BRIDGE_EXPORT(reload) int reload(void) { return 0; }
