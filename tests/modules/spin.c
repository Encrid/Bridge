/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_module.h"

TEST(init)
{
    api->wait(api, UINT64_MAX);
    api->spend(api, UINT64_MAX);
    return (0);
}
