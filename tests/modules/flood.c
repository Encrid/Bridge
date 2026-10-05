/* Spawns until the entity ceiling, and past it. */
#include "bridge-wasm.h"

BRIDGE_EXPORT(flood) int flood()
{
    uint32_t i;
    for (i = 0; i < 1000000; i ++) bridge_spawn();
    return 0;
}

BRIDGE_EXPORT(init) int init()
{
    int none = 0;
    BRIDGE_SYSTEM("flood", "update", &none, 0);
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown() { return 0; }
BRIDGE_EXPORT(reload) int reload() { return 0; }
