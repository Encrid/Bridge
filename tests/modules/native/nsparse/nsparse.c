/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/native/nsparse/nsparse.c -- a component kept in a sparse
** set, beside one kept in a table. A is in a table and every entity carries
** it; S is in a sparse set and some do.
**
**     spawn            the entities, each with A
**     attach_b         S for every second one, with a value of its own.
**                      BIG: for every one
**     detach_b         and taken from each of them again
**     detach_b_half    S taken from the first that has it
**     value_b          a value for one that has S already
**     attach_b_half    one more entity, with S and nothing else: it is in
**                      no table
**     destroy_half     one that has S destroyed, and one that has not
**     look             the set walked: a query on S alone, one span
**     look_b           the table walked, S reached from each row through
**                      the set's index
**     destroy          every one
**
** BIG is 204,800 entities and no observer, for a measurement; FAT is 10,000
** that carry 4 KB. BAD is a system the load is refused for: 1 selects by A
** and by S, 2 by not having S.
**
** storage.md, *A sparse set*
*/

#include "nsparse.h"
#include "native_idle.h"

/*
** add_changes
**
** Registers the eight systems that walk nothing, each with the component it
** attaches or detaches.
*/
static void add_changes(const bridge_api_t *api)
{
    int none = 0;

    BRIDGE_NATIVE_ADD_SYSTEM(api, "spawn", "spawn", &none, 0, &nsparse_a, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "attach_b", "attach_b", &none, 0,
        &nsparse_s, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "detach_b", "detach_b", &none, 0,
        &nsparse_s, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "detach_b_half", "detach_b_half", &none, 0,
        &nsparse_s, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "value_b", "value_b", &none, 0, &nsparse_s,
        1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "attach_b_half", "attach_b_half", &none, 0,
        &nsparse_s, 1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "destroy_half", "destroy_half", &none, 0,
        nullptr, 0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "destroy", "destroy", &none, 0, nullptr, 0);
}

/*
** add_walks
**
** Registers the system that walks the set, by S alone, and the one that
** walks the table, by A and maybe S, and keeps their queries.
*/
static void add_walks(const bridge_api_t *api)
{
    bridge_term_t one[1] = {{ nsparse_s, BRIDGE_READ, BRIDGE_AND, 0 }};
    bridge_term_t two[2] = {{ nsparse_a, BRIDGE_READ, BRIDGE_AND, 0 },
        { nsparse_s, BRIDGE_READ, BRIDGE_OPTIONAL, 0 }};

    nsparse_in_set = BRIDGE_NATIVE_ADD_SYSTEM(api, "look", "look", one, 1,
        nullptr, 0);
    nsparse_in_table = BRIDGE_NATIVE_ADD_SYSTEM(api, "look_b", "look_b", two,
        2, nullptr, 0);
}

#if !defined(BIG) && !defined(FAT)
/*
** add_observers
**
** Registers the observer of S, and the observer of A and S: an observer
** walks nothing, so its terms may be of both.
*/
static void add_observers(const bridge_api_t *api)
{
    bridge_term_t one[1] = {{ nsparse_s, BRIDGE_READ, BRIDGE_AND, 0 }};
    bridge_term_t two[2] = {{ nsparse_a, BRIDGE_READ, BRIDGE_AND, 0 },
        { nsparse_s, BRIDGE_READ, BRIDGE_AND, 0 }};

    nsparse_on_s = BRIDGE_NATIVE_OBSERVER(api, "heard_s",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, one, 1, nullptr,
        0);
    nsparse_on_both = BRIDGE_NATIVE_OBSERVER(api, "heard_both",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, two, 2, nullptr,
        0);
}

#endif

#ifdef BAD
/*
** add_refused
**
** Registers the system the load is refused for: in the build BAD 1 one that
** selects by A and by S, and in the build BAD 2 one that selects by not
** having S.
*/
static void add_refused(const bridge_api_t *api)
{
    bridge_term_t one[1] = {{ nsparse_s, BRIDGE_READ, BRIDGE_NOT, 0 }};
    bridge_term_t two[2] = {{ nsparse_a, BRIDGE_READ, BRIDGE_AND, 0 },
        { nsparse_s, BRIDGE_READ, BRIDGE_AND, 0 }};

    if (BAD == 1)
        BRIDGE_NATIVE_ADD_SYSTEM(api, "both", "never", two, 2, nullptr, 0);
    else
        BRIDGE_NATIVE_ADD_SYSTEM(api, "without", "never", one, 1, nullptr, 0);
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
    nsparse_a = BRIDGE_NATIVE_COMPONENT(api, "st:A", a_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    nsparse_s = BRIDGE_NATIVE_COMPONENT(api, "st:S", s_t, BRIDGE_DELETE,
        BRIDGE_SPARSE_SET);
    add_changes(api);
    add_walks(api);
#if !defined(BIG) && !defined(FAT)
    add_observers(api);
#endif
#ifdef BAD
    add_refused(api);
#endif
    return (0);
}
