/* Spawns past the entity ceiling. Nothing can unwind native code, so the system runs to its
 * end and the invocation is failed when it returns. */
#include "bridge_native.h"



BRIDGE_NATIVE(flood)
{
    uint32_t i;
    for (i = 0; i < 100'000; i ++) api->spawn(api);
    return 0;
}

BRIDGE_NATIVE(init)
{
    int none = 0;
    BRIDGE_NATIVE_ADD_SYSTEM(api, "flood", "update", &none, 0, nullptr, 0);
    return 0;
}

BRIDGE_NATIVE(shutdown) { return 0; }
BRIDGE_NATIVE(reload) { return 0; }
