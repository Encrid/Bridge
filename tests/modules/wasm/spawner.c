/* Spawns three entities a pass and says which ids it was given. Two of these in one level, on
 * several threads, must be given the same ids every run. */
#include "bridge_wasm.h"

BRIDGE_EXPORT(make) int make()
{
    uint32_t a = (uint32_t)bridge_spawn(), b = (uint32_t)bridge_spawn();
    BRIDGE_LOG(BRIDGE_LOG_INFO, "ids", a, b);
    BRIDGE_LOG(BRIDGE_LOG_INFO, "ids", (uint32_t)bridge_spawn(), 0);
    return 0;
}

BRIDGE_EXPORT(init) int init()
{
    int none = 0;
    BRIDGE_SYSTEM("make", "update", &none, 0, nullptr, 0);
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown() { return 0; }
BRIDGE_EXPORT(reload) int reload() { return 0; }
