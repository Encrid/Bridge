/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/nsingle/nsingle.c -- a singleton: one value for the
** deployment, on no entity.
**
**     attach_a     it is given a value, with the id 0: heard, as an
**                  entity's is
**     value_a      a value for it when it has one: heard as an attach
**     work         a field of it written where it is: no call, and nothing
**                  is heard
**     detach_b     it is taken: it has no value
**     look         what a system reads of it: whether it has a value, and
**                  the value
**     one          it read a million times, found again each time, for a
**                  measurement
**
** BAD is a build Bridge refuses something of: 1 a system that
** selects by it, at load; 2 an attach of it with the id of an entity, when
** that system runs; 3 an attach of a component that is no singleton with
** the id 0.
**
** storage.md, *A singleton*
*/

#include "nsingle.h"
#include "native_idle.h"

uint32_t nsingle_g;
uint32_t nsingle_a;
uint32_t nsingle_on_g;

/*
** add_changes
**
** Declares the two components, and registers the three systems that walk
** nothing, each with the components it attaches or detaches.
*/
static void add_changes(const bridge_api_t *api)
{
    int none = 0;
    uint32_t both[2];

    nsingle_g = BRIDGE_NATIVE_COMPONENT(api, "st:G", g_t, BRIDGE_DELETE,
        BRIDGE_SINGLETON);
    nsingle_a = BRIDGE_NATIVE_COMPONENT(api, "st:A", a_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    both[0] = nsingle_g;
    both[1] = nsingle_a;
    BRIDGE_NATIVE_ADD_SYSTEM(api, "attach_a", "attach_a", &none, 0, both, 2);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "value_a", "value_a", &none, 0, &nsingle_g,
        1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "detach_b", "detach_b", &none, 0,
        &nsingle_g, 1);
}

/*
** add_readers
**
** Registers the systems that state G as a term, and the observer of it. In
** the build BAD 1 it registers one more, which selects by G and is what the
** load is refused for.
*/
static void add_readers(const bridge_api_t *api)
{
    bridge_term_t one[1] = {{ nsingle_g, BRIDGE_READ, BRIDGE_OPTIONAL, 0 }};

    BRIDGE_NATIVE_ADD_SYSTEM(api, "look", "look", one, 1, nullptr, 0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "one", "one", one, 1, nullptr, 0);
    nsingle_which = nsingle_g;
    one[0].access = BRIDGE_WRITE;
    BRIDGE_NATIVE_ADD_SYSTEM(api, "work", "work", one, 1, nullptr, 0);
    one[0].access = BRIDGE_READ;
    one[0].oper = BRIDGE_AND;
    nsingle_on_g = BRIDGE_NATIVE_OBSERVER(api, "heard_g",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, one, 1, nullptr,
        0);
#if defined(BAD) && BAD == 1
    BRIDGE_NATIVE_ADD_SYSTEM(api, "by", "never", one, 1, nullptr, 0);
#endif
}

/*
** init
**
** Declares the singleton and the component kept in a table, and registers
** the systems and the observer.
*/
BRIDGE_NATIVE(init)
{
    add_changes(api);
    add_readers(api);
    return (0);
}
