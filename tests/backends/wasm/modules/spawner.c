/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "wasm_idle.h"

BRIDGE_EXPORT(make) int make(void)
{
    uint32_t a = (uint32_t)bridge_spawn();
    uint32_t b = (uint32_t)bridge_spawn();

    BRIDGE_LOG(BRIDGE_LOG_INFO, "ids", a, b);
    BRIDGE_LOG(BRIDGE_LOG_INFO, "ids", (uint32_t)bridge_spawn(), 0);
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    int none = 0;

    BRIDGE_ADD_SYSTEM("make", "update", &none, 0, nullptr, 0, nullptr, 0);
    return (0);
}
