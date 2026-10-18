/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_native.h"
#include "native_idle.h"

constexpr uint32_t SPAWNS = 100'000;

BRIDGE_NATIVE(flood)
{
    uint32_t i = 0;

    while (i < SPAWNS) {
        api->spawn(api);
        i++;
    }
    return (0);
}

BRIDGE_NATIVE(init)
{
    int none = 0;

    BRIDGE_NATIVE_ADD_SYSTEM(api, "flood", "update", &none, 0, nullptr, 0);
    return (0);
}
