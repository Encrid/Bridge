/* Declares a struct with a field of st:Vec, a type another module declares: its rows hold a
 * Vec as it is, so no reload of that other module may lay Vec out again.
 * docs/bridge/data/lifetime.md, *When a component layout changed*. */
#include "bridge_native.h"

/* declared by the module loaded before this one: here it is only what Y is made of */
typedef struct { uint16_t x, y; } Vec;
#define Y_FIELDS { Vec at; }
BRIDGE_SCHEMA_STRUCT_TYPE(Y, Y_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(Y, Y_FIELDS);

BRIDGE_NATIVE(init)
{
    BRIDGE_NATIVE_COMPONENT(api, "st:Y", Y, BRIDGE_DELETE, BRIDGE_TABLE);
    return 0;
}

BRIDGE_NATIVE(reload) { return 0; }
BRIDGE_NATIVE(shutdown) { return 0; }
