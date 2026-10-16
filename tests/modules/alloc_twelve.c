/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "say.h"

TEST(init)
{
    say(api, "alloc", api->alloc(api, 12), 0);
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
