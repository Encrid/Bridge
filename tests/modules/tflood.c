/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_module.h"
#include "idle.h"

constexpr uint32_t SPAWNS = 100'000;

TEST(flood)
{
    uint32_t i = 0;

    while (i < SPAWNS) {
        api->spawn(api);
        i++;
    }
    return (0);
}

TEST(init)
{
    int none = 0;

    TEST_ADD_SYSTEM(api, "flood", "update", &none, 0, nullptr, 0, nullptr, 0);
    return (0);
}
