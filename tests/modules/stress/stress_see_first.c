/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "stress.h"

TEST(see0)
{
    return (nstress_check_kind(api, 0));
}

TEST(see1)
{
    return (nstress_check_kind(api, 1));
}

TEST(see2)
{
    return (nstress_check_kind(api, 2));
}

TEST(see3)
{
    return (nstress_check_kind(api, 3));
}

TEST(see4)
{
    return (nstress_check_kind(api, 4));
}
