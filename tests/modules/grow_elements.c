/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_module.h"

TEST(init)
{
    return (api->grow_elements(api, 1u << 16) ? 0 : 7);
}

TEST(shutdown)
{
    return (0);
}

TEST(reload)
{
    return (0);
}
