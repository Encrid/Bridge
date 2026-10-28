/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_module.h"
#include "spot.h"

TEST(init)
{
    uint32_t name = test_put(api, "five:Spot", 9);
    uint32_t desc = test_put(api, spot_t_desc, sizeof(spot_t_desc) - 1);

    api->declare(api, TEST_STATE(api, bridge_declaration_t, name, 9,
        BRIDGE_STRUCT, (uint32_t)sizeof(spot_t) + EXTRA,
        (uint32_t)alignof(spot_t), desc, sizeof(spot_t_desc) - 1,
        BRIDGE_DELETE, BRIDGE_TABLE, BRIDGE_ON_DESTROY_REMOVE));
    return (0);
}

TEST(reload)
{
    return (0);
}

TEST(shutdown)
{
    return (0);
}
