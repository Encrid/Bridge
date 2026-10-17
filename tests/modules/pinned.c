/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
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

TEST(one)
{
    return (say_thread(api));
}

TEST(two)
{
    return (say_thread(api));
}

TEST(three)
{
    return (say_thread(api));
}

TEST(init)
{
    int none = 0;

    TEST_ADD_SYSTEM(api, "one", "update", &none, 0, nullptr, 0, nullptr, 0);
    TEST_ADD_SYSTEM(api, "two", "update", &none, 0, nullptr, 0, nullptr, 0);
    TEST_ADD_SYSTEM(api, "three", "present", &none, 0, nullptr, 0, nullptr, 0);
    return (0);
}
