/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_module.h"
#include "idle.h"
#include "platform.h"

TEST(nap)
{
    bridge_line_t line = { "nap", 3, 0, get_now_ms() };

    sleep_tenth();
    line.a0 = get_thread_id();
    test_log(api, BRIDGE_LOG_INFO, &line);
    line.text = "end";
    line.a0 = get_thread_id();
    line.a1 = get_now_ms();
    test_log(api, BRIDGE_LOG_INFO, &line);
    return (0);
}

TEST(init)
{
    int none = 0;

    TEST_ADD_SYSTEM(api, "nap", "update", &none, 0, nullptr, 0);
    return (0);
}
