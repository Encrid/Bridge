/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "wasm_idle.h"

constexpr uint32_t SPAWNS = 1'000'000;

BRIDGE_EXPORT(flood) int flood(void)
{
    uint32_t i = 0;

    while (i < SPAWNS) {
        bridge_spawn();
        i++;
    }
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    int none = 0;

    BRIDGE_ADD_SYSTEM("flood", "update", &none, 0, nullptr, 0, nullptr, 0);
    return (0);
}
