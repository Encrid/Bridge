/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/modules/native/nstore/nstore.c -- storage, exercised and told
** about: every structural change a module can ask for, each in a phase of
** its own so that a host can make one and then look, and what a system and
** an observer see afterwards. It says what it saw in its log and decides
** nothing.
**
** Built three ways. Plain: a handful of entities, with observers, for what
** storage does. BIG: many entities and no observer, for what it costs.
** TABLES: ten thousand tables. And BIG again with A made larger, FAT64 to
** FAT4K.
**
** stage10.md, *What flecs does, pinned*
*/

#include "nstore.h"
#include "native/native_measured.h"
#include "native_idle.h"

/*
** spawn
**
** Spawns the entities, with no component.
*/
NATIVE_MEASURED BRIDGE_NATIVE(spawn)
{
    uint32_t i = 0;

    while (i < COUNT) {
        nstore_ids[i] = api->spawn(api);
        i++;
    }
    return (0);
}

/*
** add_changes
**
** Registers the six systems every build has. What a system attaches or
** detaches is stated with it.
*/
static void add_changes(const bridge_api_t *api)
{
    int none = 0;
    uint32_t both[2] = { nstore_a, nstore_b };

    BRIDGE_NATIVE_ADD_SYSTEM(api, "spawn", "spawn", &none, 0, nullptr, 0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "attach_a", "attach_a", &none, 0, &nstore_a,
        1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "attach_b", "attach_b", &none, 0, &nstore_b,
        1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "detach_b", "detach_b", &none, 0, &nstore_b,
        1);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "destroy", "destroy", &none, 0, nullptr, 0);
    BRIDGE_NATIVE_ADD_SYSTEM(api, "attach_both", "attach_both", &none, 0,
        both, 2);
}

/*
** init
**
** Declares A and B, and registers the systems and the observers the build
** has.
*/
BRIDGE_NATIVE(init)
{
    nstore_a = BRIDGE_NATIVE_COMPONENT(api, "st:A", a_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    nstore_b = BRIDGE_NATIVE_COMPONENT(api, "st:B", b_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    add_changes(api);
#ifdef TABLES
    nstore_add_tables(api);
#endif
#if !defined(BIG) && !defined(TABLES)
    nstore_add_plain(api);
#endif
    return (0);
}
