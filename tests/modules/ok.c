/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/ok.c -- a module that loads, and does nothing.
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

/*
** reload
**
** Answers 0.
*/
TEST(reload)
{
    return (0);
}
