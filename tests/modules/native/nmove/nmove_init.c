/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/native/nmove/nmove_init.c -- one component, X, kept the way
** this build states: KEPT is the number of the storage layout. A module is
** loaded as one build and reloaded as another, and X's values are to be
** what they were.
**
**     spawn          six entities with A, and X on every second one: 100,
**                    102, 104
**     detach_b       X taken from two of the three, so that one entity
**                    carries it
**     detach_b_half  and from all three, so that none does
**     flicker        a toggle: the row whose value is 102 turned off
**     look           who carries X and what the values come to, read as
**                    this build keeps it
**
** lifetime.md, *When a component is moved*
*/

#include "nmove.h"

constexpr uint8_t ASKED = KEPT >= 4 ? BRIDGE_OPTIONAL : BRIDGE_AND;

uint32_t a_slot;
uint32_t x_slot;
uint32_t reading;
uint64_t ids[COUNT];

/*
** init
**
** Declares A, and X kept as KEPT says, and the five systems. A component
** kept direct-indexed or as a singleton is in no table, so the term that
** names it does not ask for it.
*/
BRIDGE_NATIVE(init)
{
    int none = 0;
    bridge_term_t one[1] = {{ 0, BRIDGE_WRITE, ASKED, 0 }};
    uint32_t both[2];

    a_slot = BRIDGE_NATIVE_COMPONENT(api, "st:A", a_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    x_slot = BRIDGE_NATIVE_COMPONENT(api, "st:X", x_t, BRIDGE_DELETE,
        (enum bridge_storage)KEPT);
    both[0] = a_slot;
    both[1] = x_slot;
    BRIDGE_NATIVE_ADD_SYSTEM(api, "spawn", "spawn", &none, 0, both, 2);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "detach_b", "detach_b", &none, 0, &x_slot,
        1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "detach_b_half", "detach_b_half", &none, 0,
        &x_slot, 1);
    one[0].slot = x_slot;
    reading = BRIDGE_NATIVE_ADD_SYSTEM(api, "look", "look", one, 1, nullptr,
        0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "flicker", "flicker", one, 1, nullptr, 0);
    return (0);
}

/*
** reload
**
** Declares everything again, as init does.
*/
BRIDGE_NATIVE(reload)
{
    return (bridge_init(api));
}

/*
** shutdown
**
** Does nothing.
*/
BRIDGE_NATIVE(shutdown)
{
    return (0);
}
