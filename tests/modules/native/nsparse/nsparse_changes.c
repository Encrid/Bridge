/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/modules/native/nsparse/nsparse_changes.c -- the systems of nsparse
** that change every entity, or every one that is given S.
*/

#include "nsparse.h"
#include "native/native_measured.h"

uint32_t nsparse_a;
uint32_t nsparse_s;
uint64_t nsparse_ids[COUNT];

/*
** spawn
**
** Spawns the entities, each with A.
*/
BRIDGE_NATIVE(spawn)
{
    a_t value = {};

    while (value.v < COUNT) {
        nsparse_ids[value.v] = api->spawn(api);
        BRIDGE_NATIVE_ATTACH(api, nsparse_ids[value.v], nsparse_a, value);
        value.v++;
    }
    return (0);
}

/*
** attach_b
**
** Gives S to every second entity, or in a build for a measurement to every
** one, each with a value of its own.
*/
NATIVE_MEASURED BRIDGE_NATIVE(attach_b)
{
    s_t value;
    uint32_t i = 0;

    while (i < COUNT) {
        value.v = 100 + i;
        BRIDGE_NATIVE_ATTACH(api, nsparse_ids[i], nsparse_s, value);
        i += STEP;
    }
    return (0);
}

/*
** detach_b
**
** Takes S from each entity `attach_b` gave it to.
*/
NATIVE_MEASURED BRIDGE_NATIVE(detach_b)
{
    uint32_t i = 0;

    while (i < COUNT) {
        api->detach(api, nsparse_ids[i], nsparse_s);
        i += STEP;
    }
    return (0);
}

/*
** destroy
**
** Destroys every entity, and the one more that has S and nothing else if
** it was made.
*/
BRIDGE_NATIVE(destroy)
{
    uint32_t i = 0;

    while (i < COUNT) {
        api->destroy(api, nsparse_ids[i]);
        i++;
    }
    if (nsparse_lone)
        api->destroy(api, nsparse_lone);
    return (0);
}
