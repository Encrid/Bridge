/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "single.h"

TEST(attach_a)
{
    g_t value = { 10, 1 };
    [[maybe_unused]] a_t other = { 1 };

#if defined(BAD) && BAD == 2
    TEST_ATTACH(api, api->spawn(api), nsingle_g, value);
#elif defined(BAD) && BAD == 3
    TEST_ATTACH(api, 0, nsingle_a, other);
#endif
    TEST_ATTACH(api, 0, nsingle_g, value);
    return (0);
}

TEST(value_a)
{
    g_t value = { 20, 2 };

    TEST_ATTACH(api, 0, nsingle_g, value);
    return (0);
}

TEST(work)
{
    g_t *value = test_get_single(api, nsingle_g);

    if (value)
        value->wind = 77;
    return (0);
}

TEST(flicker)
{
    g_t value = { 30, 3 };

    TEST_ATTACH(api, 0, nsingle_g, value);
    api->detach(api, 0, nsingle_g);
    return (0);
}

TEST(detach_b)
{
    api->detach(api, 0, nsingle_g);
    return (0);
}
