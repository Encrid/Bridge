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
    uint32_t slot = TEST_COMPONENT(api, "test:Spare", spare_t,
        BRIDGE_DELETE, BRIDGE_TABLE);
    bridge_term_t term = { slot, BRIDGE_READ, BRIDGE_AND, 0 };
    spare_t value = { 1 };

#ifdef SPANNED
    TEST_ATTACH(api, api->spawn(api), slot, value);
    TEST_ADD_SYSTEM(api, "tick", "update", &term, 1, nullptr, 0);
#else
    (void)value;
    TEST_ADD_SYSTEM(api, "tick", "update", &term, 0, nullptr, 0);
#endif
    return (0);
}
