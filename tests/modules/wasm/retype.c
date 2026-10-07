/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/wasm/retype.c -- declares one component name with a layout
** nobody else gives it. It loads only if nothing of that name is left: how
** a test sees that a component is gone. Built once per name, which WHICH
** states.
*/

#include "bridge_wasm.h"
#include "retype.h"
#include "wasm_idle.h"

/*
** init
**
** Declares the name the build states, with the layout of retype.h.
*/
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
