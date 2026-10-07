/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/modules/native/ndirect/ndirect.c -- a direct-indexed component
** beside one kept in a table. A is in a table and every entity carries it;
** D is direct-indexed and some have been given one.
**
**     spawn            the entities, each with A
**     attach_b         D for every second one, with a value of its own.
**                      BIG: for every one
**     detach_b         and taken from each of them again
**     detach_b_half    D taken from the first that has it
**     value_b          a value for one that has D already
**     destroy_half     one that has D destroyed, and one that has not
**     respawn_half     two more, with A alone: they are given the places of
**                      the two that went, and none of what was there
**     look_b           the table walked, D reached at each row by the
**                      entity's id
**     near, far        each entity reached by an id that is held, in the
**                      order they lie and in no order, with the id's
**                      generation tested
**     destroy          every one
**
** BIG is 204,800 entities and no observer, for a measurement. BAD is a
** system the load is refused for: it selects by D.
**
** storage.md, *Direct-indexed*
*/

#include "ndirect.h"
#include "native_idle.h"

/*
** add_changes
**
** Registers the eight systems that state no term, each with the component
** it attaches or detaches.
*/
static void add_changes(const bridge_api_t *api)
{
    int none = 0;

    BRIDGE_NATIVE_ADD_SYSTEM(api, "spawn", "spawn", &none, 0, &ndirect_a, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "respawn_half", "respawn_half", &none, 0,
        &ndirect_a, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "attach_b", "attach_b", &none, 0,
        &ndirect_d, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "detach_b", "detach_b", &none, 0,
        &ndirect_d, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "detach_b_half", "detach_b_half", &none, 0,
        &ndirect_d, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "value_b", "value_b", &none, 0, &ndirect_d,
        1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "destroy_half", "destroy_half", &none, 0,
        nullptr, 0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "destroy", "destroy", &none, 0, nullptr, 0);
}

/*
** add_readers
**
** Registers the system that walks the table, by A and maybe D, and keeps
** its query; and the two that reach D by ids. Those two walk nothing: the
** term is how they say what they read.
*/
static void add_readers(const bridge_api_t *api)
{
    bridge_term_t two[2] = {{ ndirect_a, BRIDGE_READ, BRIDGE_AND, 0 },
        { ndirect_d, BRIDGE_READ, BRIDGE_OPTIONAL, 0 }};

    ndirect_in_table = BRIDGE_NATIVE_ADD_SYSTEM(api, "look_b", "look_b", two,
        2, nullptr, 0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "near", "near", &two[1], 1, nullptr, 0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "far", "far", &two[1], 1, nullptr, 0);
}

#ifndef BIG
/*
** add_observers
**
** Registers the observer of D, and the observer of A and D.
*/
static void add_observers(const bridge_api_t *api)
{
    bridge_term_t one[1] = {{ ndirect_d, BRIDGE_READ, BRIDGE_AND, 0 }};
    bridge_term_t two[2] = {{ ndirect_a, BRIDGE_READ, BRIDGE_AND, 0 },
        { ndirect_d, BRIDGE_READ, BRIDGE_AND, 0 }};

    ndirect_on_d = BRIDGE_NATIVE_OBSERVER(api, "heard_d",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, one, 1, nullptr,
        0);
    ndirect_on_both = BRIDGE_NATIVE_OBSERVER(api, "heard_both",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, two, 2, nullptr,
        0);
}

#endif

#ifdef BAD
/*
** add_refused
**
** Registers the system the load is refused for: it selects by A and by D.
*/
static void add_refused(const bridge_api_t *api)
{
    bridge_term_t two[2] = {{ ndirect_a, BRIDGE_READ, BRIDGE_AND, 0 },
        { ndirect_d, BRIDGE_READ, BRIDGE_AND, 0 }};

    BRIDGE_NATIVE_ADD_SYSTEM(api, "both", "never", two, 2, nullptr, 0);
}

#endif

/*
** init
**
** Declares the two components, and registers the systems, the observers a
** build has, and the system a build is refused for.
*/
BRIDGE_NATIVE(init)
{
    ndirect_a = BRIDGE_NATIVE_COMPONENT(api, "st:A", a_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    ndirect_d = BRIDGE_NATIVE_COMPONENT(api, "st:D", d_t, BRIDGE_DELETE,
        BRIDGE_DIRECT_INDEXED);
    add_changes(api);
    add_readers(api);
#ifndef BIG
    add_observers(api);
#endif
#ifdef BAD
    add_refused(api);
#endif
    return (0);
}
