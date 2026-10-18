/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "nlayout.h"

constexpr uint8_t ASKED = KEPT >= 4 ? BRIDGE_OPTIONAL : BRIDGE_AND;

uint32_t x_slot;
uint32_t reading;

BRIDGE_NATIVE(init)
{
    int none = 0;
    bridge_term_t one[1] = {{ 0, BRIDGE_READ, ASKED, 0 }};

    BRIDGE_NATIVE_DECLARE(api, BRIDGE_ENUM, "st:switch_t", switch_t,
        BRIDGE_STRIP, BRIDGE_TABLE);
#if V == 13
    BRIDGE_NATIVE_DECLARE_TAG(api, "st:vec_t", BRIDGE_STRIP, BRIDGE_TABLE);
#else
    BRIDGE_NATIVE_DECLARE(api, BRIDGE_STRUCT, "st:vec_t", vec_t,
        BRIDGE_STRIP, BRIDGE_TABLE);
#endif
    x_slot = BRIDGE_NATIVE_COMPONENT(api, "st:X", x_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "spawn", "spawn", &none, 0, &x_slot, 1);
    one[0].slot = x_slot;
    reading = BRIDGE_NATIVE_ADD_SYSTEM(api, "look", "look", one, 1, nullptr,
        0);
    return (0);
}

BRIDGE_NATIVE(reload)
{
    return (bridge_init(api));
}

BRIDGE_NATIVE(shutdown)
{
    return (0);
}
