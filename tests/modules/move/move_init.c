/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "move.h"

constexpr uint8_t ASKED = KEPT >= 4 ? BRIDGE_OPTIONAL : BRIDGE_AND;

uint32_t a_slot;
uint32_t x_slot;
uint32_t reading;
uint64_t ids[COUNT];

TEST(init)
{
    int none = 0;
    bridge_term_t one[1] = {{ 0, BRIDGE_WRITE, ASKED, 0 }};
    uint32_t both[2];

    a_slot = TEST_COMPONENT(api, "st:A", a_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    x_slot = TEST_COMPONENT(api, "st:X", x_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    both[0] = a_slot;
    both[1] = x_slot;
    TEST_ADD_SYSTEM(api, "spawn", "spawn", &none, 0, both, 2, nullptr, 0);
    TEST_ADD_SYSTEM(api, "detach_b", "detach_b", &none, 0, &x_slot,
        1, nullptr, 0);
    TEST_ADD_SYSTEM(api, "detach_b_half", "detach_b_half", &none, 0,
        &x_slot, 1, nullptr, 0);
    one[0].slot = x_slot;
    reading = TEST_ADD_SYSTEM(api, "look", "look", one, 1, nullptr,
        0, nullptr, 0);
    TEST_ADD_SYSTEM(api, "flicker", "flicker", one, 1, nullptr, 0, nullptr, 0);
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
