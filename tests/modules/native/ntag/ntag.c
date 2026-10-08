/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/ntag/ntag.c -- tags: components with no bytes, one
** kept in each storage layout. A is a component of four bytes that every
** entity carries.
**
**     spawn        six entities with A. M, in tables, on every second; P,
**                  in a sparse set, on the first two; O, a toggle, on every
**                  one; R, direct-indexed, on the odd
**     flicker      O turned off for the first three, by their bits
**     attach_a     W, a singleton, given
**     detach_b     M taken from every one that has it: one table again
**     look         what a module is handed for each
**
** storage.md, *A tag*
*/

#include "ntag.h"
#include "native_idle.h"

uint32_t ntag_a;
uint32_t ntag_m;
uint32_t ntag_p;
uint32_t ntag_o;
uint32_t ntag_r;
uint32_t ntag_w;
uint32_t ntag_marked;
uint32_t ntag_in_set;
uint32_t ntag_toggled;
uint64_t ntag_ids[COUNT];

/*
** declare_components
**
** Declares A, and a tag kept in each storage layout.
*/
static void declare_components(const bridge_api_t *api)
{
    ntag_a = BRIDGE_NATIVE_COMPONENT(api, "st:A", a_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    ntag_m = BRIDGE_NATIVE_DECLARE_TAG(api, "st:M", BRIDGE_DELETE,
        BRIDGE_TABLE);
    ntag_p = BRIDGE_NATIVE_DECLARE_TAG(api, "st:P", BRIDGE_DELETE,
        BRIDGE_SPARSE_SET);
    ntag_o = BRIDGE_NATIVE_DECLARE_TAG(api, "st:O", BRIDGE_DELETE,
        BRIDGE_TOGGLE);
    ntag_r = BRIDGE_NATIVE_DECLARE_TAG(api, "st:R", BRIDGE_DELETE,
        BRIDGE_DIRECT_INDEXED);
    ntag_w = BRIDGE_NATIVE_DECLARE_TAG(api, "st:W", BRIDGE_DELETE,
        BRIDGE_SINGLETON);
}

/*
** add_changes
**
** Registers the three systems that walk nothing, each with the components
** it attaches or detaches.
*/
static void add_changes(const bridge_api_t *api)
{
    int none = 0;
    uint32_t made[5] = { ntag_a, ntag_m, ntag_p, ntag_o, ntag_r };

    BRIDGE_NATIVE_ADD_SYSTEM(api, "spawn", "spawn", &none, 0, made, 5);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "attach_a", "attach_a", &none, 0, &ntag_w,
        1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "detach_b", "detach_b", &none, 0, &ntag_m,
        1);
}

/*
** add_walks
**
** Registers the three systems that walk, and keeps the query of each.
*/
static void add_walks(const bridge_api_t *api)
{
    bridge_term_t two[2] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 },
        { 0, BRIDGE_READ, BRIDGE_AND, 0 }};

    two[0].slot = ntag_a;
    two[1].slot = ntag_m;
    ntag_marked = BRIDGE_NATIVE_ADD_SYSTEM(api, "look", "look", two, 2,
        nullptr, 0);
    two[0].slot = ntag_p;
    ntag_in_set = BRIDGE_NATIVE_ADD_SYSTEM(api, "in_set", "never", two, 1,
        nullptr, 0);
    two[0].slot = ntag_a;
    two[1].slot = ntag_o;
    two[1].access = BRIDGE_WRITE;
    ntag_toggled = BRIDGE_NATIVE_ADD_SYSTEM(api, "flicker", "flicker", two, 2,
        nullptr, 0);
}

/*
** init
**
** Declares A and the five tags, and registers the systems.
*/
BRIDGE_NATIVE(init)
{
    declare_components(api);
    add_changes(api);
    add_walks(api);
    return (0);
}

/*
** in_set
**
** Does nothing. It is registered in a phase no host runs, and exists for
** the spans its query is given, which `look` reads.
*/
BRIDGE_NATIVE(in_set)
{
    return (0);
}
