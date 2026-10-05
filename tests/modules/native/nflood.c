/* Spawns past the entity ceiling. Nothing can unwind native code, so the system runs to its
 * end and the invocation is failed when it returns. */
#include "bridge-native.h"

static const bridge_api *api;

BRIDGE_NATIVE_BIND(api)

BRIDGE_NATIVE(flood)
{
    uint32_t i;
    for (i = 0; i < 100000; i ++) api->spawn();
    return 0;
}

BRIDGE_NATIVE(init)
{
    int none = 0;
    BRIDGE_NATIVE_SYSTEM(api, "flood", "update", &none, 0);
    return 0;
}

BRIDGE_NATIVE(shutdown) { return 0; }
BRIDGE_NATIVE(reload) { return 0; }
