/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/noinit.c -- a module with no init: what it has is called
** something else.
*/

#include "test_module.h"

/*
** start
**
** Answers 0. Nothing asks for it.
*/
TEST(start)
{
    return (0);
}
