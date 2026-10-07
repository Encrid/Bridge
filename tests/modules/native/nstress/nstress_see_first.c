/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/native/nstress/nstress_see_first.c -- the systems of nstress
** that see its first five components, each through a query of its own.
*/

#include "nstress.h"

/*
** see0
**
** Checks the first component against the account.
*/
BRIDGE_NATIVE(see0)
{
    return (nstress_check_kind(api, 0));
}

/*
** see1
**
** Checks the second component against the account.
*/
BRIDGE_NATIVE(see1)
{
    return (nstress_check_kind(api, 1));
}

/*
** see2
**
** Checks the third component against the account.
*/
BRIDGE_NATIVE(see2)
{
    return (nstress_check_kind(api, 2));
}

/*
** see3
**
** Checks the fourth component against the account.
*/
BRIDGE_NATIVE(see3)
{
    return (nstress_check_kind(api, 3));
}

/*
** see4
**
** Checks the fifth component against the account.
*/
BRIDGE_NATIVE(see4)
{
    return (nstress_check_kind(api, 4));
}
