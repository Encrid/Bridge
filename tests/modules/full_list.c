/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "idle.h"

    #define V6_FIELDS { uint32_t v; }

BRIDGE_SCHEMA_STRUCT_TYPE(v6_t, V6_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(v6_t, V6_FIELDS);

constexpr uint32_t COUNT6 = 2000;

static uint32_t slot6;

TEST(fill)
{
    uint64_t id;
    uint32_t i = 0;

    while (i < COUNT6) {
        id = api->spawn(api);
        api->attach(api, id, slot6, BRIDGE_ZEROED);
        i++;
    }
    return (0);
}

TEST(look)
{
    return (0);
}

TEST(heard)
{
    return (0);
}

TEST(init)
{
    int none = 0;
    bridge_term_t a[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};

    slot6 = TEST_COMPONENT(api, "st:A", v6_t, BRIDGE_DELETE, BRIDGE_TABLE);
    a[0].slot = slot6;
    TEST_ADD_SYSTEM(api, "fill", "spawn", &none, 0, &slot6, 1);
    TEST_ADD_SYSTEM(api, "look", "look", a, 1, nullptr, 0);
    TEST_OBSERVER(api, "heard", BRIDGE_ON_ENTER, a, 1, nullptr, 0);
    return (0);
}
