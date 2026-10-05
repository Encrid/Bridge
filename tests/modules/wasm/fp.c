/* Declares game:Position with another struct body: built against an older header, say. */
#include "bridge-wasm.h"

BRIDGE_SCHEMA_STRUCT(Position, { float x; float y; float z; });

BRIDGE_EXPORT(init) int init()
{
    BRIDGE_COMPONENT("game:Position", Position, BRIDGE_STRIP);
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown() { return 0; }
BRIDGE_EXPORT(reload) int reload() { return 0; }
