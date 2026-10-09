/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/sleep.c -- a system that takes a tenth of a second
** or more, and says which thread it was on, when it began and when it
** ended. Two of these whose times overlap ran at once.
*/

#include "test_module.h"
#include "idle.h"
#include "platform.h"

/*
** nap
**
** Sleeps, and says when it began and when it ended, in milliseconds, each
** with the thread it was on: two ran at once if one began before the other
** ended. How long a nap takes is not asked: a sleep is a least time, and
** some systems return from a tenth of a second after two.
*/
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

/*
** init
**
** Declares the one system.
*/
TEST(init)
{
    int none = 0;

    TEST_ADD_SYSTEM(api, "nap", "update", &none, 0, nullptr, 0);
    return (0);
}
