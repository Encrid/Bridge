/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "retype.h"
#include "wasm_idle.h"

BRIDGE_EXPORT(init) int init(void)
{
#if WHICH == 1
    BRIDGE_COMPONENT("game:Private", retyped_t, BRIDGE_STRIP, BRIDGE_TABLE);
#elif WHICH == 2
    BRIDGE_COMPONENT("game:vec2_t", retyped_t, BRIDGE_STRIP, BRIDGE_TABLE);
#elif WHICH == 4
    BRIDGE_COMPONENT("game:Settings", retyped_t, BRIDGE_STRIP, BRIDGE_TABLE);
#else
    BRIDGE_COMPONENT("game:Extra", retyped_t, BRIDGE_STRIP, BRIDGE_TABLE);
#endif
    return (0);
}
