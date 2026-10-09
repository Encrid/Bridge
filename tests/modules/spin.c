/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/spin.c -- a module whose init never ends by itself: it
** waits, and works, without end, and returns only where nothing bounds it.
*/

#include "test_module.h"

/*
** init
**
** Waits longer than any timeout, and does more work than any limit.
*/
TEST(init)
{
    api->wait(api, UINT64_MAX);
    api->spend(api, UINT64_MAX);
    return (0);
}
