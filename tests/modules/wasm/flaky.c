/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/wasm/flaky.c -- a system that fails FAILS passes out of
** every OF. Built four ways: always, one in three, two in three, and always
** beside a system of the next phase that never fails, which CALM adds.
**
** failure.md, *The budget*
*/

#include "bridge_wasm.h"
#include "wasm_idle.h"

static uint32_t passes;

/*
** trip
**
** Says which pass it is and whether it fails, and answers 7 when it does.
*/
BRIDGE_EXPORT(trip) int trip(void)
{
    uint32_t n = passes % OF;

    passes++;
    BRIDGE_LOG(BRIDGE_LOG_INFO, "trip", passes, n < FAILS);
    return (n < FAILS ? 7 : 0);
}

#ifdef CALM

/*
** calm
**
** Never fails.
*/
BRIDGE_EXPORT(calm) int calm(void)
{
    return (0);
}

#endif

/*
** init
**
** Registers the system that fails and, in the build that has it, the one
** that never does.
*/
BRIDGE_EXPORT(init) int init(void)
{
    int none = 0;

    BRIDGE_ADD_SYSTEM("trip", "update", &none, 0, nullptr, 0);
#ifdef CALM
    BRIDGE_ADD_SYSTEM("calm", "present", &none, 0, nullptr, 0);
#endif
    return (0);
}
