/* Names an observer init, which is an export that already means something. */
#include "bridge-wasm.h"
#include "../game.h"

BRIDGE_EXPORT(init) int init()
{
    bridge_term terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    terms[0].slot = BRIDGE_COMPONENT("game:Position", Position, BRIDGE_STRIP);
    BRIDGE_OBSERVER("init", BRIDGE_ON_ADD, terms, 1);
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown() { return 0; }
BRIDGE_EXPORT(reload) int reload() { return 0; }
