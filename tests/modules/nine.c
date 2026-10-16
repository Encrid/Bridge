/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "say.h"
#include "nine.h"

static uint32_t settings;

static void declare_nine(const test_api_t *api)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_OPTIONAL, 0 }};

    settings = TEST_COMPONENT(api, "nine:Settings", nine_t, BRIDGE_DELETE,
        BRIDGE_SINGLETON);
    terms[0].slot = settings;
    TEST_ADD_SYSTEM(api, "show", "update", terms, 1, nullptr, 0);
}

TEST(show)
{
    const nine_t *v = test_get_single(api, settings);

    say(api, "width", v ? (uint32_t)v->width : 0, 0);
    return (0);
}

TEST(init)
{
    declare_nine(api);
    return (0);
}

TEST(reload)
{
    declare_nine(api);
    return (0);
}

TEST(shutdown)
{
    return (0);
}
