/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_module.h"

TEST(init)
{
#ifdef GHOST
    int none = 0;

    TEST_ADD_SYSTEM(api, "ghost", "update", &none, 0, nullptr, 0, nullptr, 0);
#endif
    return (0);
}

TEST(reload)
{
    return (0);
}
