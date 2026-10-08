/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/nstore/nstore_plain.c -- what only the plain build
** of nstore registers: the changes of some entities, what a system sees,
** and the observers.
*/

#include "nstore.h"

#if !defined(BIG) && !defined(TABLES)
/*
** add_first_changes
**
** Registers the first seven of the systems that state no term, each with
** the components it attaches or detaches.
*/
static void add_first_changes(const bridge_api_t *api)
{
    int none = 0;
    uint32_t both[2] = { nstore_a, nstore_b };

    BRIDGE_NATIVE_ADD_SYSTEM(api, "attach_b_half", "attach_b_half", &none, 0,
        &nstore_b, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "detach_b_half", "detach_b_half", &none, 0,
        &nstore_b, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "detach_b_absent", "detach_b_absent", &none,
        0, &nstore_b, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "value_a", "value_a", &none, 0, &nstore_a,
        1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "flicker", "flicker", &none, 0, both, 2);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "destroy_half", "destroy_half", &none, 0,
        nullptr, 0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "respawn_half", "respawn_half", &none, 0,
        nullptr, 0);
}

/*
** add_last_changes
**
** Registers the last six of the systems that state no term, each with the
** component it attaches or detaches.
*/
static void add_last_changes(const bridge_api_t *api)
{
    int none = 0;

    BRIDGE_NATIVE_ADD_SYSTEM(api, "touch_dead", "touch_dead", &none, 0,
        &nstore_a, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "recycled", "recycled", &none, 0, nullptr,
        0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "attach_a_half", "attach_a_half", &none, 0,
        &nstore_a, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "scribble", "scribble", &none, 0, nullptr,
        0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "find", "find", &none, 0, nullptr, 0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "value_b", "value_b", &none, 0, &nstore_b,
        1);
}

/*
** add_looks
**
** Registers the system that looks, the two whose queries it reads, which
** are in a phase no host runs, and the observer of A; and keeps the query
** or the list of each.
*/
static void add_looks(const bridge_api_t *api)
{
    bridge_term_t one[1] = {{ nstore_a, BRIDGE_READ, BRIDGE_AND, 0 }};
    bridge_term_t two[2] = {{ nstore_a, BRIDGE_READ, BRIDGE_AND, 0 },
        { nstore_b, BRIDGE_READ, BRIDGE_NOT, 0 }};

    nstore_with_a = BRIDGE_NATIVE_ADD_SYSTEM(api, "look", "look", one, 1,
        nullptr, 0);
    nstore_without_b = BRIDGE_NATIVE_ADD_SYSTEM(api, "look_not", "never", two,
        2, nullptr, 0);
    two[1].oper = BRIDGE_OPTIONAL;
    nstore_maybe_b = BRIDGE_NATIVE_ADD_SYSTEM(api, "look_maybe", "never", two,
        2, nullptr, 0);
    nstore_on_a = BRIDGE_NATIVE_OBSERVER(api, "heard_a",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, one, 1, nullptr,
        0);
}

/*
** add_hearers
**
** Registers the system that looks at B and the observers that state B, and
** keeps the query or the list of each. An event is about the terms and not
** about the change: the observer of A and not B enters at a detach.
*/
static void add_hearers(const bridge_api_t *api)
{
    bridge_term_t one[1] = {{ nstore_b, BRIDGE_READ, BRIDGE_AND, 0 }};
    bridge_term_t two[2] = {{ nstore_a, BRIDGE_READ, BRIDGE_AND, 0 },
        { nstore_b, BRIDGE_READ, BRIDGE_AND, 0 }};

    nstore_with_b = BRIDGE_NATIVE_ADD_SYSTEM(api, "look_b", "look_b", one, 1,
        nullptr, 0);
    nstore_on_b = BRIDGE_NATIVE_OBSERVER(api, "heard_b",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, one, 1, nullptr,
        0);
    nstore_on_both = BRIDGE_NATIVE_OBSERVER(api, "heard_both",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, two, 2, nullptr,
        0);
    two[1].oper = BRIDGE_NOT;
    nstore_on_alone = BRIDGE_NATIVE_OBSERVER(api, "heard_alone",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, two, 2, nullptr,
        0);
}

/*
** nstore_add_plain
**
** Registers the systems and the observers that only the plain build has.
*/
void nstore_add_plain(const bridge_api_t *api)
{
    add_first_changes(api);
    add_last_changes(api);
    add_looks(api);
    add_hearers(api);
}

#endif
