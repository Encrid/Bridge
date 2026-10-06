/* Declares game:Position as the mover does, and states a storage layout for it that is unset,
 * or one that is no storage layout. storage.md, *Storage layouts*. */
#include "bridge-wasm.h"
#include "../game.h"

BRIDGE_EXPORT(init) int init()
{
    BRIDGE_COMPONENT("game:Position", Position, BRIDGE_DELETE, (enum bridge_storage)KEPT);
    return 0;
}

BRIDGE_EXPORT(shutdown) int shutdown() { return 0; }
BRIDGE_EXPORT(reload) int reload() { return 0; }
