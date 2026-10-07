/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/modules/native/nsparse/nsparse_half.c -- the systems of nsparse
** that change one entity or two.
*/

#include "nsparse.h"

uint64_t nsparse_lone;

/*
** detach_b_half
**
** Takes S from the first entity that has it.
*/
BRIDGE_NATIVE(detach_b_half)
{
    api->detach(api, nsparse_ids[0], nsparse_s);
    return (0);
}

/*
** value_b
**
** Gives a value to an entity that has S already.
*/
BRIDGE_NATIVE(value_b)
{
    s_t value = { 900 };

    BRIDGE_NATIVE_ATTACH(api, nsparse_ids[2 % COUNT], nsparse_s, value);
    return (0);
}

/*
** attach_b_half
**
** Spawns one more entity, with S and nothing else: it is in no table.
*/
BRIDGE_NATIVE(attach_b_half)
{
    s_t value = { 7 };

    nsparse_lone = api->spawn(api);
    BRIDGE_NATIVE_ATTACH(api, nsparse_lone, nsparse_s, value);
    return (0);
}

/*
** destroy_half
**
** Destroys an entity that has S, and one that has not.
*/
BRIDGE_NATIVE(destroy_half)
{
    api->destroy(api, nsparse_ids[2 % COUNT]);
    api->destroy(api, nsparse_ids[1 % COUNT]);
    return (0);
}
