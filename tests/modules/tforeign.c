/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
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

TEST(init)
{
    int none = 0;

    TEST_ADD_SYSTEM(api, "reach", "update", &none, 0, nullptr, 0, nullptr, 0);
    return (0);
}
