/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_schema.h"
#include "test_module.h"

#if V == 2
    #define HUGE_FIELDS { uint32_t v; uint8_t big[1073741820]; }
#elif V == 3
    #define HUGE_FIELDS { uint8_t big[524288]; }
#else
    #define HUGE_FIELDS { uint32_t v; }
#endif
#if V == 3
    #define HUGE_KEPT BRIDGE_DIRECT_INDEXED
#else
    #define HUGE_KEPT BRIDGE_TABLE
#endif

BRIDGE_SCHEMA_STRUCT_TYPE(huge_t, HUGE_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(huge_t, HUGE_FIELDS);

static uint32_t huge;

TEST(reload)
{
    huge = TEST_COMPONENT(api, "test:Huge", huge_t, BRIDGE_DELETE,
        HUGE_KEPT);
    return (0);
}

TEST(init)
{
    (void)TEST_OWN(reload)(api);
#if V == 1
    api->attach(api, api->spawn(api), huge, BRIDGE_ZEROED);
    api->attach(api, api->spawn(api), huge, BRIDGE_ZEROED);
#endif
    return (0);
}

TEST(shutdown)
{
    return (0);
}
