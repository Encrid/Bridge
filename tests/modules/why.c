/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "say.h"

TEST(init)
{
    say(api, "init wrote this", 7, 0);
    return (INIT_STATUS);
}

TEST(reload)
{
    say(api, "reload wrote this", 5, 0);
    return (5);
}

TEST(shutdown)
{
    return (0);
}
