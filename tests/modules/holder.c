/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_module.h"
#include "idle.h"
#include "holder.h"

TEST(init)
{
    TEST_COMPONENT(api, "st:Y", y_t, BRIDGE_DELETE, BRIDGE_TABLE);
    return (0);
}
