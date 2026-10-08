/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/wasm/spin3.c -- one system never returns, and another must
** still run after it was stopped.
*/

#include "bridge_wasm.h"
#include "wasm_idle.h"

static volatile int going = 1;

/*
** forever
**
** Never returns.
*/
BRIDGE_EXPORT(forever) int forever(void)
{
    while (going)
        continue;
    return (0);
}

/*
** after
**
** Says that it ran.
*/
BRIDGE_EXPORT(after) int after(void)
{
    BRIDGE_LOG(BRIDGE_LOG_INFO, "still here", 0, 0);
    return (0);
}

/*
** init
**
** Registers the system that never returns, and the one of the next phase.
*/
BRIDGE_EXPORT(init) int init(void)
{
    int none = 0;

    BRIDGE_ADD_SYSTEM("forever", "update", &none, 0, nullptr, 0);
    BRIDGE_ADD_SYSTEM("after", "present", &none, 0, nullptr, 0);
    return (0);
}
