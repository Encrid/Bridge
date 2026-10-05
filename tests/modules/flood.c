/* Spawns until the entity ceiling, and past it. */
#include "bridge-wasm.h"

BRIDGE_EXPORT(flood) int flood(void)
{
    uint32_t i;
    for (i = 0; i < 1000000; i ++) bridge_spawn();
    return 0;
}

BRIDGE_EXPORT(init) int init(void)
{
    int none = 0;
    BRIDGE_SYSTEM("flood", "update", &none, 0);
    return 0;
}
