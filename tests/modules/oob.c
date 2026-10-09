/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/oob.c -- a module whose init cannot go on: the call is
** ended, as a backend ends a module that faults where it can catch one.
*/

#include "test_module.h"

/*
** init
**
** Ends the call.
*/
TEST(init)
{
    api->trap(api);
    return (0);
}
