/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/nstore/nstore_never.c -- the systems of the plain
** build of nstore that are registered in a phase no host runs: they exist
** for the spans their queries are given, which `look` reads.
*/

#include "nstore.h"

#if !defined(BIG) && !defined(TABLES)
/*
** look_not
**
** Does nothing. Its query is with A and not B.
*/
BRIDGE_NATIVE(look_not)
{
    return (0);
}

/*
** look_maybe
**
** Does nothing. Its query is with A and maybe B.
*/
BRIDGE_NATIVE(look_maybe)
{
    return (0);
}

#endif
