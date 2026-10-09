/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/stray.c -- a module that imports a thing nobody answers,
** which is not a memory and not a function. It says so by what it exports,
** since a library has no list of imports for a backend to read.
*/

#include "test_module.h"

/*
** test_import_stray
**
** Is there to be found: the tests' backend takes a module that exports it
** to import what nobody answers.
*/
TEST_EXPORT void test_import_stray(void)
{
}

/*
** init
**
** Answers 0. It is never reached.
*/
TEST(init)
{
    return (0);
}
