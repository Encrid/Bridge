/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_module.h"
#include "idle.h"

    #define SPARE_FIELDS { uint32_t v; }

BRIDGE_SCHEMA_STRUCT_TYPE(spare_t, SPARE_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(spare_t, SPARE_FIELDS);

TEST(tick)
{
    return (0);
}

TEST(init)
{
    int none = 0;

    (void)TEST_COMPONENT(api, "test:Spare", spare_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    TEST_ADD_SYSTEM(api, "tick", "update", &none, 0, nullptr, 0);
    return (0);
}
