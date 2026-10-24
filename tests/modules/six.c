/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "say.h"
#include "six.h"

static uint32_t six;

#ifndef SEEDED
static void hold_one(const test_api_t *api)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_OPTIONAL, 0 }};
    six_t one = { 1 };

    terms[0].slot = six;
    TEST_ATTACH(api, 0, six, one);
    TEST_ADD_SYSTEM(api, "show", "update", terms, 1, nullptr, 0, nullptr, 0);
}

TEST(show)
{
    const six_t *v = test_get_single(api, six);

    say(api, "x", v ? v->x : 0, 0);
    return (0);
}

TEST(reload)
{
    return (0);
}
#endif

TEST(init)
{
    six = TEST_COMPONENT(api, "six:Settings", six_t, BRIDGE_DELETE,
        BRIDGE_SINGLETON);
#ifndef SEEDED
    hold_one(api);
#endif
    return (0);
}

TEST(shutdown)
{
    return (0);
}
