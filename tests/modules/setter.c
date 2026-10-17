/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_module.h"
#include "game.h"
#include "idle.h"

constexpr uint32_t ENTITIES = 3;

static uint32_t position;
static uint32_t marked;
static uint32_t runs;
static uint64_t ids[ENTITIES];

TEST(mark)
{
    marked_t m = { 77 };
    bridge_line_t line = { "marked", 6, 0, 0 };

    runs++;
    if (runs != 1)
        return (0);
    TEST_ATTACH(api, ids[1], marked, m);
    test_log(api, BRIDGE_LOG_INFO, &line);
    return (0);
}

TEST(init)
{
    int none = 0;
    position_t p = { 0.0f, 0.0f };
    uint32_t i = 0;

    position = TEST_COMPONENT(api, "game:Position", position_t,
        BRIDGE_DELETE, BRIDGE_TABLE);
    marked = TEST_COMPONENT(api, "game:Marked", marked_t,
        BRIDGE_STRIP, BRIDGE_TABLE);
    TEST_ADD_SYSTEM(api, "mark", "update", &none, 0, &marked, 1, nullptr, 0);
    while (i < ENTITIES) {
        p.x = (float)i;
        ids[i] = api->spawn(api);
        TEST_ATTACH(api, ids[i], position, p);
        i++;
    }
    return (0);
}
