/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "vec2.h"
#include "wasm_idle.h"

BRIDGE_EXPORT(init) int init(void)
{
    BRIDGE_COMPONENT("game:vec2_t", vec2_t, BRIDGE_STRIP, BRIDGE_TABLE);
    return (0);
}
