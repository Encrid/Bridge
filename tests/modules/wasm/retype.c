/* Declares one component name with a layout nobody else gives it. It loads only if nothing of
 * that name is left: how a test sees that a component is gone. Built once per name. */
#include "bridge-wasm.h"

BRIDGE_SCHEMA_STRUCT(Retyped, { uint32_t a; uint32_t b; uint32_t c; });

BRIDGE_EXPORT(init) int init()
{
#if WHICH == 1
    BRIDGE_COMPONENT("game:Private", Retyped, BRIDGE_STRIP, BRIDGE_TABLE);
#elif WHICH == 2
    BRIDGE_COMPONENT("game:Vec2", Retyped, BRIDGE_STRIP, BRIDGE_TABLE);
#elif WHICH == 4
    BRIDGE_COMPONENT("game:Settings", Retyped, BRIDGE_STRIP, BRIDGE_TABLE);
#else
    BRIDGE_COMPONENT("game:Extra", Retyped, BRIDGE_STRIP, BRIDGE_TABLE);
#endif
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown() { return 0; }
BRIDGE_EXPORT(reload) int reload() { return 0; }
