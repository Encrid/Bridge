/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "weigh.h"
#include "idle.h"

uint32_t nweigh_flag;
uint32_t nweigh_link;
uint32_t nweigh_single;
static uint64_t ids[COUNT];
static uint32_t state = 2463534242u;

static uint32_t draw_number(void)
{
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return (state);
}

static void fill_row(const test_api_t *api, uint32_t i)
{
    v_t v = { 1 };
    f_t f = { draw_number() % ON == 0, 1 };
    l_t l = { ids[i], ids[draw_number() % COUNT] };

    TEST_ATTACH(api, ids[i], nweigh_value, v);
    TEST_ATTACH(api, ids[i], nweigh_flag, f);
    TEST_ATTACH(api, ids[i], nweigh_link, l);
}

TEST(fill)
{
    uint64_t holder = api->spawn(api);
    s_t s = { 1 };
    uint32_t i = 0;

    while (i < COUNT) {
        ids[i] = api->spawn(api);
        i++;
    }
    i = 0;
    while (i < COUNT) {
        fill_row(api, i);
        i++;
    }
    TEST_ATTACH(api, holder, nweigh_single, s);
    return (0);
}

static void add_readers(const test_api_t *api)
{
    bridge_term_t one[1] = {{ nweigh_value, BRIDGE_READ, BRIDGE_AND, 0 }};

    nweigh_walking = TEST_ADD_SYSTEM(api, "walk", "walk", one, 1,
        nullptr, 0, nullptr, 0);
    one[0].slot = nweigh_flag;
    nweigh_flagged = TEST_ADD_SYSTEM(api, "flag", "flag", one, 1,
        nullptr, 0, nullptr, 0);
    one[0].slot = nweigh_link;
    nweigh_linked = TEST_ADD_SYSTEM(api, "near", "near", one, 1,
        nullptr, 0, nullptr, 0);
    TEST_ADD_SYSTEM(api, "far", "far", one, 1, nullptr, 0, nullptr, 0);
    one[0].slot = nweigh_single;
    nweigh_which = TEST_ADD_SYSTEM(api, "one", "one", one, 1,
        nullptr, 0, nullptr, 0);
}

TEST(init)
{
    int none = 0;
    uint32_t filled[4];

    nweigh_value = TEST_COMPONENT(api, "st:V", v_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    nweigh_flag = TEST_COMPONENT(api, "st:F", f_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    nweigh_link = TEST_COMPONENT(api, "st:L", l_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    nweigh_single = TEST_COMPONENT(api, "st:S", s_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    filled[0] = nweigh_value;
    filled[1] = nweigh_flag;
    filled[2] = nweigh_link;
    filled[3] = nweigh_single;
    TEST_ADD_SYSTEM(api, "fill", "spawn", &none, 0, filled, 4, nullptr, 0);
    add_readers(api);
    return (0);
}
