/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/native/nsingle/nsingle_changes.c -- the systems of nsingle
** that give the singleton a value, write it and take it.
*/

#include "nsingle.h"

/*
** attach_a
**
** Gives the singleton a value, with the id 0. In the build BAD 2 it first
** attaches it with the id of an entity, and in the build BAD 3 it first
** attaches a component that is no singleton with the id 0.
*/
BRIDGE_NATIVE(attach_a)
{
    g_t value = { 10, 1 };
    [[maybe_unused]] a_t other = { 1 };

#if defined(BAD) && BAD == 2
    BRIDGE_NATIVE_ATTACH(api, api->spawn(api), nsingle_g, value);
#elif defined(BAD) && BAD == 3
    BRIDGE_NATIVE_ATTACH(api, 0, nsingle_a, other);
#endif
    BRIDGE_NATIVE_ATTACH(api, 0, nsingle_g, value);
    return (0);
}

/*
** value_a
**
** Gives the singleton another value, when it has one.
*/
BRIDGE_NATIVE(value_a)
{
    g_t value = { 20, 2 };

    BRIDGE_NATIVE_ATTACH(api, 0, nsingle_g, value);
    return (0);
}

/*
** work
**
** Writes a field of the singleton where it is, with no call.
*/
BRIDGE_NATIVE(work)
{
    g_t *value = bridge_native_get_single(api, nsingle_g);

    if (value)
        value->wind = 77;
    return (0);
}

/*
** detach_b
**
** Takes the singleton's value.
*/
BRIDGE_NATIVE(detach_b)
{
    api->detach(api, 0, nsingle_g);
    return (0);
}
