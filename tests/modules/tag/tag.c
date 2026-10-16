/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "tag.h"
#include "idle.h"

uint32_t ntag_a;
uint32_t ntag_m;
uint32_t ntag_p;
uint32_t ntag_o;
uint32_t ntag_r;
uint32_t ntag_w;
uint32_t ntag_marked;
uint32_t ntag_in_set;
uint32_t ntag_toggled;
uint64_t ntag_ids[COUNT];

static void declare_components(const test_api_t *api)
{
    ntag_a = TEST_COMPONENT(api, "st:A", a_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    ntag_m = TEST_DECLARE_TAG(api, "st:M", BRIDGE_DELETE,
        BRIDGE_TABLE);
    ntag_p = TEST_DECLARE_TAG(api, "st:P", BRIDGE_DELETE,
        BRIDGE_SPARSE_SET);
    ntag_o = TEST_DECLARE_TAG(api, "st:O", BRIDGE_DELETE,
        BRIDGE_TOGGLE);
    ntag_r = TEST_DECLARE_TAG(api, "st:R", BRIDGE_DELETE,
        BRIDGE_DIRECT_INDEXED);
    ntag_w = TEST_DECLARE_TAG(api, "st:W", BRIDGE_DELETE,
        BRIDGE_SINGLETON);
}

static void add_changes(const test_api_t *api)
{
    int none = 0;
    uint32_t made[5] = { ntag_a, ntag_m, ntag_p, ntag_o, ntag_r };

    TEST_ADD_SYSTEM(api, "spawn", "spawn", &none, 0, made, 5);
    TEST_ADD_SYSTEM(api, "attach_a", "attach_a", &none, 0, &ntag_w,
        1);
    TEST_ADD_SYSTEM(api, "detach_b", "detach_b", &none, 0, &ntag_m,
        1);
}

static void add_walks(const test_api_t *api)
{
    bridge_term_t two[2] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 },
        { 0, BRIDGE_READ, BRIDGE_AND, 0 }};

    two[0].slot = ntag_a;
    two[1].slot = ntag_m;
    ntag_marked = TEST_ADD_SYSTEM(api, "look", "look", two, 2,
        nullptr, 0);
    two[0].slot = ntag_p;
    ntag_in_set = TEST_ADD_SYSTEM(api, "in_set", "never", two, 1,
        nullptr, 0);
    two[0].slot = ntag_a;
    two[1].slot = ntag_o;
    two[1].access = BRIDGE_WRITE;
    ntag_toggled = TEST_ADD_SYSTEM(api, "flicker", "flicker", two, 2,
        nullptr, 0);
}

TEST(init)
{
    declare_components(api);
    add_changes(api);
    add_walks(api);
    return (0);
}

TEST(in_set)
{
    return (0);
}
