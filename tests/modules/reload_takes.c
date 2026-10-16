/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "say.h"

static void take_id_and_bytes(const test_api_t *api, const char *what)
{
    uint64_t id = api->spawn(api);
    uint32_t at = api->alloc(api, 8);

    say(api, what, (uint32_t)id, at);
}

TEST(init)
{
    return (0);
}

TEST(reload)
{
    take_id_and_bytes(api, "reload took");
    return (0);
}

TEST(shutdown)
{
    take_id_and_bytes(api, "shutdown took");
    return (0);
}
