/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/grow.c -- a module whose init asks for four megabytes
** more memory of its own, and answers 7 if it is refused them.
*/

#include "test_module.h"

/*
** init
**
** Answers 7 where the growth was refused, and 0 where it was not.
*/
TEST(init)
{
    return (api->grow(api, 64u << 16) ? 0 : 7);
}
