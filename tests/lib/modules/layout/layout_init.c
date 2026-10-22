/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "layout.h"

constexpr uint8_t ASKED = KEPT >= 4 ? BRIDGE_OPTIONAL : BRIDGE_AND;

uint32_t x_slot;
uint32_t reading;

TEST(init)
{
    int none = 0;
    bridge_term_t one[1] = {{ 0, BRIDGE_READ, ASKED, 0 }};

    TEST_DECLARE(api, BRIDGE_ENUM, "st:switch_t", switch_t,
        BRIDGE_STRIP, BRIDGE_TABLE);
#if V == 13
    TEST_DECLARE_TAG(api, "st:vec_t", BRIDGE_STRIP, BRIDGE_TABLE);
#else
    TEST_DECLARE(api, BRIDGE_STRUCT, "st:vec_t", vec_t,
        BRIDGE_STRIP, BRIDGE_TABLE);
#endif
#if V == 16
    TEST_DECLARE(api, BRIDGE_STRUCT, "st:other_t", other_t,
        BRIDGE_STRIP, BRIDGE_TABLE);
#endif
    x_slot = TEST_COMPONENT(api, "st:X", x_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
#if V == 20
    x_slot = TEST_COMPONENT(api, "st:X", x_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
#elif V == 21
    x_slot = TEST_COMPONENT(api, "st:X", x3_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
#endif
    TEST_ADD_SYSTEM(api, "spawn", "spawn", &none, 0, &x_slot, 1, nullptr, 0);
    one[0].slot = x_slot;
    reading = TEST_ADD_SYSTEM(api, "look", "look", one, 1, nullptr,
        0, nullptr, 0);
    return (0);
}

TEST(reload)
{
    return (TEST_OWN(init)(api));
}

TEST(shutdown)
{
    return (0);
}
