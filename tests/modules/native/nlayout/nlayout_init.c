/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/nlayout/nlayout_init.c -- one component, X, whose
** struct this build states: V says which. A module is loaded as V 0 and
** reloaded as another, and what its values become is what is tested. KEPT
** is the storage layout X is kept in, the same in both.
**
**     spawn    three entities with X: a 1000.., b 20.., arr 1 2 3 4 and up,
**              at 5 6 and up, ON
**     look     every field this build has, summed over the values there are
**
** lifetime.md, *When a component layout changed*
*/

#include "nlayout.h"

constexpr uint8_t ASKED = KEPT >= 4 ? BRIDGE_OPTIONAL : BRIDGE_AND;

uint32_t x_slot;
uint32_t reading;

/*
** init
**
** Declares the enum and the struct that X's fields name, then X kept as
** KEPT says, and the two systems. The build whose V is 13 declares the
** struct as a tag. A component kept direct-indexed or as a singleton is in
** no table, so the term that names it does not ask for it.
*/
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
