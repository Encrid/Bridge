/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/pinned.c -- three systems that each say which
** thread they were on. Loaded pinned, it must be one.
*/

#include "test_module.h"
#include "idle.h"
#include "platform.h"

static int say_thread(const test_api_t *api)
{
    bridge_line_t line = { "pin", 3, get_thread_id(), 0 };

    test_log(api, BRIDGE_LOG_INFO, &line);
    return (0);
}

/*
** one
**
** Says which thread it is on.
*/
TEST(one)
{
    return (say_thread(api));
}

/*
** two
**
** Says which thread it is on.
*/
TEST(two)
{
    return (say_thread(api));
}

/*
** three
**
** Says which thread it is on.
*/
TEST(three)
{
    return (say_thread(api));
}

/*
** init
**
** Declares the three systems: two in one phase, and one in another.
*/
TEST(init)
{
    int none = 0;

    TEST_ADD_SYSTEM(api, "one", "update", &none, 0, nullptr, 0);
    TEST_ADD_SYSTEM(api, "two", "update", &none, 0, nullptr, 0);
    TEST_ADD_SYSTEM(api, "three", "present", &none, 0, nullptr, 0);
    return (0);
}
