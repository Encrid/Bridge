/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/stress/stress_see_last.c -- the systems of nstress
** that see its last three components, each through a query of its own.
*/

#include "stress.h"

/*
** see5
**
** Checks the sixth component against the account.
*/
TEST(see5)
{
    return (nstress_check_kind(api, 5));
}

/*
** see6
**
** Checks the seventh component against the account.
*/
TEST(see6)
{
    return (nstress_check_kind(api, 6));
}

/*
** see7
**
** Checks the eighth component against the account.
*/
TEST(see7)
{
    return (nstress_check_kind(api, 7));
}
