/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/modules/native/nstress/nstress_see_last.c -- the systems of nstress
** that see its last three components, each through a query of its own.
*/

#include "nstress.h"

/*
** see5
**
** Checks the sixth component against the account.
*/
BRIDGE_NATIVE(see5)
{
    return (nstress_check_kind(api, 5));
}

/*
** see6
**
** Checks the seventh component against the account.
*/
BRIDGE_NATIVE(see6)
{
    return (nstress_check_kind(api, 6));
}

/*
** see7
**
** Checks the eighth component against the account.
*/
BRIDGE_NATIVE(see7)
{
    return (nstress_check_kind(api, 7));
}
