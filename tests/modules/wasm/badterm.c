/* Hands over a term whose operator is none of the three there are: its load is refused. */
#include "bridge-wasm.h"
#include "../game.h"

BRIDGE_EXPORT(any) int any() { return 0; }

BRIDGE_EXPORT(init) int init()
{
    bridge_term terms[1] = {{ 0, BRIDGE_READ, (enum bridge_oper)4, 0 }};
    terms[0].slot = BRIDGE_COMPONENT("game:Position", Position, BRIDGE_STRIP);
    BRIDGE_SYSTEM("any", "update", terms, 1, nullptr, 0);
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown() { return 0; }
BRIDGE_EXPORT(reload) int reload() { return 0; }
