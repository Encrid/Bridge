/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/stress/stress_see_first.c -- the systems of nstress
** that see its first five components, each through a query of its own.
*/

#include "stress.h"

/*
** see0
**
** Checks the first component against the account.
*/
TEST(see0)
{
    return (nstress_check_kind(api, 0));
}

/*
** see1
**
** Checks the second component against the account.
*/
TEST(see1)
{
    return (nstress_check_kind(api, 1));
}

/*
** see2
**
** Checks the third component against the account.
*/
TEST(see2)
{
    return (nstress_check_kind(api, 2));
}

/*
** see3
**
** Checks the fourth component against the account.
*/
TEST(see3)
{
    return (nstress_check_kind(api, 3));
}

/*
** see4
**
** Checks the fifth component against the account.
*/
TEST(see4)
{
    return (nstress_check_kind(api, 4));
}
