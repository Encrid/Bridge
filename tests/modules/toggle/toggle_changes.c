/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "toggle.h"

uint32_t ntoggle_a;
uint32_t ntoggle_b;
uint32_t ntoggle_t;
uint64_t ntoggle_ids[2 * COUNT];
uint32_t ntoggle_more;

static void spawn_entities(const test_api_t *api, uint32_t from)
{
    a_t first;
    t_t second;
    uint32_t i = from;

    while (i < from + COUNT) {
        first.v = i;
        second.v = 10 + i;
        ntoggle_ids[i] = api->spawn(api);
        TEST_ATTACH(api, ntoggle_ids[i], ntoggle_a, first);
        TEST_ATTACH(api, ntoggle_ids[i], ntoggle_t, second);
        i++;
    }
}

TEST(spawn)
{
    spawn_entities(api, 0);
    return (0);
}

TEST(respawn_half)
{
    spawn_entities(api, COUNT);
    ntoggle_more = COUNT;
    return (0);
}

TEST(attach_b)
{
    b_t value = {};

    while (value.v < COUNT / 2) {
        TEST_ATTACH(api, ntoggle_ids[value.v], ntoggle_b, value);
        value.v++;
    }
    return (0);
}

TEST(destroy_half)
{
    api->destroy(api, ntoggle_ids[0]);
    api->destroy(api, ntoggle_ids[COUNT - 1]);
    return (0);
}
