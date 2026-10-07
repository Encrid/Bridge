/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/native/ntoggle/ntoggle.c -- a toggled component: a column
** of a table and a bit a row. A and B are kept in tables and T is a toggle;
** every entity carries A and T.
**
**     spawn            the entities, each with A and T: a toggle is on
**                      when it is given
**     flicker          the rows whose T is even turned off, by a write of
**                      their bits
**     combine          every row turned on again
**     look             the rows that are on, counted a word of bits at a
**                      time
**     look_b           and by a test at every row, which must agree
**     find             the same, each entity reached by its id
**     attach_b         B for the first half: they move to another table, as
**                      they were
**     destroy_half     the first and the last destroyed: another row takes
**                      each place
**     respawn_half     as many again, so that the table is moved to grow
**     value_a          a value for one that is off: it stays off
**
** BIG is 204,800 entities with one in ON of them on, in no order, for a
** measurement.
**
** storage.md, *A toggle*
*/

#include "ntoggle.h"
#include "native_idle.h"

#ifndef BIG
static uint32_t on_t;

/*
** heard_t
**
** Says what the observer of T heard: how many, and the events of the first.
** It hears an entity given T, and never a row turned on or off.
*/
BRIDGE_NATIVE(heard_t)
{
    const bridge_list_t *list = bridge_native_get_list(api, on_t);
    const bridge_trigger_t *trigger;

    trigger = (const bridge_trigger_t *)(api->private_base + list->at);
    if (list->count)
        say(api, "heard of T: how many, and the events of the first",
            list->count, trigger[0].events);
    return (0);
}

#endif

/*
** add_changes
**
** Registers the six systems that state no term, each with the components it
** attaches or detaches.
*/
static void add_changes(const bridge_api_t *api)
{
    int none = 0;
    uint32_t made[2] = { ntoggle_a, ntoggle_t };

    BRIDGE_NATIVE_ADD_SYSTEM(api, "spawn", "spawn", &none, 0, made, 2);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "respawn_half", "respawn_half", &none, 0,
        made, 2);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "attach_b", "attach_b", &none, 0,
        &ntoggle_b, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "destroy_half", "destroy_half", &none, 0,
        nullptr, 0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "value_a", "value_a", &none, 0, &ntoggle_t,
        1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "find", "find", &none, 0, nullptr, 0);
}

/*
** add_walks
**
** Registers the observer of T, which a build for a measurement does not
** have, the two systems that read T and the two that write it, and keeps a
** query of each kind.
*/
static void add_walks(const bridge_api_t *api)
{
    bridge_term_t one[1] = {{ ntoggle_t, BRIDGE_READ, BRIDGE_AND, 0 }};

#ifndef BIG
    on_t = BRIDGE_NATIVE_OBSERVER(api, "heard_t",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, one, 1, nullptr,
        0);
#endif
    ntoggle_reading = BRIDGE_NATIVE_ADD_SYSTEM(api, "look", "look", one, 1,
        nullptr, 0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "look_b", "look_b", one, 1, nullptr, 0);
    one[0].access = BRIDGE_WRITE;
    ntoggle_writing = BRIDGE_NATIVE_ADD_SYSTEM(api, "flicker", "flicker", one,
        1, nullptr, 0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "combine", "combine", one, 1, nullptr, 0);
}

/*
** init
**
** Declares the three components, and registers the systems and the
** observer.
*/
BRIDGE_NATIVE(init)
{
    ntoggle_a = BRIDGE_NATIVE_COMPONENT(api, "st:A", a_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    ntoggle_b = BRIDGE_NATIVE_COMPONENT(api, "st:B", b_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    ntoggle_t = BRIDGE_NATIVE_COMPONENT(api, "st:T", t_t, BRIDGE_DELETE,
        BRIDGE_TOGGLE);
    add_changes(api);
    add_walks(api);
    return (0);
}
