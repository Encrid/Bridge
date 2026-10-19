/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "wasm_idle.h"

constexpr uint32_t LINES = 100;

BRIDGE_EXPORT(talk) int talk(void)
{
    uint32_t i = 0;

    while (i < LINES) {
        BRIDGE_LOG(BRIDGE_LOG_INFO, "line", i, 0);
        i++;
    }
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    int none = 0;

    BRIDGE_ADD_SYSTEM("talk", "update", &none, 0, nullptr, 0, nullptr, 0);
    return (0);
}
