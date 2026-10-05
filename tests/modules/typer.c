/* Declares a struct other modules' components use as a field's type, and nothing else. */
#include "bridge-wasm.h"

ECS_STRUCT(Vec2, { float x; float y; });

BRIDGE_EXPORT(init) int init(void)
{
    BRIDGE_COMPONENT("game:Vec2", Vec2, BRIDGE_STRIP);
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown(void) { return 0; }
BRIDGE_EXPORT(reload) int reload(void) { return 0; }
