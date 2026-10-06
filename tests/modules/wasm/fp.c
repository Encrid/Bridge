/* Declares game:Position with another struct body: built against an older header, say. */
#include "bridge_wasm.h"

#define POSITION_FIELDS { float x; float y; float z; }
BRIDGE_SCHEMA_STRUCT_TYPE(Position, POSITION_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(Position, POSITION_FIELDS);

BRIDGE_EXPORT(init) int init()
{
    BRIDGE_COMPONENT("game:Position", Position, BRIDGE_STRIP, BRIDGE_TABLE);
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown() { return 0; }
BRIDGE_EXPORT(reload) int reload() { return 0; }
