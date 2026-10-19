/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "wasm_idle.h"

static uint32_t passes;

BRIDGE_EXPORT(trip) int trip(void)
{
    uint32_t n = passes % OF;

    passes++;
    BRIDGE_LOG(BRIDGE_LOG_INFO, "trip", passes, n < FAILS);
    return (n < FAILS ? 7 : 0);
}

#ifdef CALM

BRIDGE_EXPORT(calm) int calm(void)
{
    return (0);
}

#endif

BRIDGE_EXPORT(init) int init(void)
{
    int none = 0;

    BRIDGE_ADD_SYSTEM("trip", "update", &none, 0, nullptr, 0, nullptr, 0);
#ifdef CALM
    BRIDGE_ADD_SYSTEM("calm", "present", &none, 0, nullptr, 0, nullptr, 0);
#endif
    return (0);
}
