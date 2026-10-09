/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/tforeign.c -- an entry reached from a thread the
** backend did not call on answers zero, and one reached on the thread it
** did call on is honoured.
**
** signatures.md, *A call takes nothing and answers a status*
*/

#include "test_module.h"
#include "idle.h"
#include "thread.h"

static const test_api_t *kept;
static uint64_t from_elsewhere;

static void spawn_elsewhere(void)
{
    from_elsewhere = kept->spawn(kept);
}

/*
** reach
**
** Keeps the table it was handed, has another thread spawn through it, waits
** for that thread, and then spawns itself. It says whether each got an id.
*/
TEST(reach)
{
    bridge_line_t line = {
        "an id from another thread, and from this one", 44, 0, 0
    };

    kept = api;
    run_on_another_thread(spawn_elsewhere);
    line.a0 = from_elsewhere != 0;
    line.a1 = api->spawn(api) != 0;
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

    TEST_ADD_SYSTEM(api, "reach", "update", &none, 0, nullptr, 0);
    return (0);
}
