/* Asks for a term whose type changes per table, which has no typed accessor. */
#include "bridge-wasm.h"
#include "../game.h"

BRIDGE_EXPORT(any) int any() { return 0; }

BRIDGE_EXPORT(init) int init()
{
    bridge_term terms[1] = {{ BRIDGE_WILDCARD, BRIDGE_IN, BRIDGE_AND, 0 }};
    BRIDGE_COMPONENT("game:Position", Position, BRIDGE_STRIP);
    BRIDGE_SYSTEM("any", "update", terms, 1);
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown() { return 0; }
BRIDGE_EXPORT(reload) int reload() { return 0; }
