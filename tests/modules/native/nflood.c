/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/nflood.c -- spawns past the entity ceiling. Nothing
** can unwind native code, so the system runs to its end and the invocation
** is failed when it returns.
*/

#include "bridge_native.h"
#include "native_idle.h"

constexpr uint32_t SPAWNS = 100'000;

/*
** flood
**
** Spawns more entities than there may be.
*/
BRIDGE_NATIVE(flood)
{
    uint32_t i = 0;

    while (i < SPAWNS) {
        api->spawn(api);
        i++;
    }
    return (0);
}

/*
** init
**
** Declares the one system.
*/
BRIDGE_NATIVE(init)
{
    int none = 0;

    BRIDGE_NATIVE_ADD_SYSTEM(api, "flood", "update", &none, 0, nullptr, 0);
    return (0);
}
