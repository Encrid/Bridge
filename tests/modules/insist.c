/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_schema.h"
#include "test_module.h"

#define BARE_FIELDS { uint32_t v; }

BRIDGE_SCHEMA_STRUCT_TYPE(bare_t, BARE_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(bare_t, BARE_FIELDS);

TEST_EXPORT void test_enter_after_refusal(void)
{
}

TEST(look)
{
    return (0);
}

TEST(init)
{
    bridge_term_t term = { 0, BRIDGE_READ, BRIDGE_AND, 0 };

    term.slot = TEST_COMPONENT(api, "test:Bare", bare_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    (void)TEST_COMPONENT(api, "Bare", bare_t, BRIDGE_DELETE, BRIDGE_TABLE);
    TEST_ADD_SYSTEM(api, "look", "update", &term, 1, nullptr, 0, nullptr, 0);
    return (0);
}
