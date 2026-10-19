/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "game.h"
#include "wasm_idle.h"

BRIDGE_EXPORT(init) int init(void)
{
    BRIDGE_COMPONENT("game:Position", position_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    return (0);
}
