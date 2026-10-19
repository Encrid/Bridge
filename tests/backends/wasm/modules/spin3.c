/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "wasm_idle.h"

static volatile int going = 1;

BRIDGE_EXPORT(forever) int forever(void)
{
    while (going)
        continue;
    return (0);
}

BRIDGE_EXPORT(after) int after(void)
{
    BRIDGE_LOG(BRIDGE_LOG_INFO, "still here", 0, 0);
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    int none = 0;

    BRIDGE_ADD_SYSTEM("forever", "update", &none, 0, nullptr, 0, nullptr, 0);
    BRIDGE_ADD_SYSTEM("after", "present", &none, 0, nullptr, 0, nullptr, 0);
    return (0);
}
