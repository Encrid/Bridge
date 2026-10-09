/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/noreload.c -- a module with an init and a shutdown, and
** no reload.
*/

#include "test_module.h"

/*
** init
**
** Answers 0.
*/
TEST(init)
{
    return (0);
}

/*
** shutdown
**
** Answers 0.
*/
TEST(shutdown)
{
    return (0);
}
