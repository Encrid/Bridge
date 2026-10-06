/* Declares a struct other modules' components use as a field's type, and nothing else. */
#include "bridge_wasm.h"

#define VEC2_FIELDS { float x; float y; }
BRIDGE_SCHEMA_STRUCT_TYPE(Vec2, VEC2_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(Vec2, VEC2_FIELDS);

BRIDGE_EXPORT(init) int init()
{
    BRIDGE_COMPONENT("game:Vec2", Vec2, BRIDGE_STRIP, BRIDGE_TABLE);
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown() { return 0; }
BRIDGE_EXPORT(reload) int reload() { return 0; }
