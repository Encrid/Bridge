/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "direct.h"
#include "idle.h"

static void add_changes(const test_api_t *api)
{
    int none = 0;

    TEST_ADD_SYSTEM(api, "spawn", "spawn", &none, 0, &ndirect_a, 1);
    TEST_ADD_SYSTEM(api, "respawn_half", "respawn_half", &none, 0,
        &ndirect_a, 1);
    TEST_ADD_SYSTEM(api, "attach_b", "attach_b", &none, 0,
        &ndirect_d, 1);
    TEST_ADD_SYSTEM(api, "detach_b", "detach_b", &none, 0,
        &ndirect_d, 1);
    TEST_ADD_SYSTEM(api, "detach_b_half", "detach_b_half", &none, 0,
        &ndirect_d, 1);
    TEST_ADD_SYSTEM(api, "value_b", "value_b", &none, 0, &ndirect_d,
        1);
    TEST_ADD_SYSTEM(api, "destroy_half", "destroy_half", &none, 0,
        nullptr, 0);
    TEST_ADD_SYSTEM(api, "destroy", "destroy", &none, 0, nullptr, 0);
}

static void add_readers(const test_api_t *api)
{
    bridge_term_t two[2] = {{ ndirect_a, BRIDGE_READ, BRIDGE_AND, 0 },
        { ndirect_d, BRIDGE_READ, BRIDGE_OPTIONAL, 0 }};

    ndirect_in_table = TEST_ADD_SYSTEM(api, "look_b", "look_b", two,
        2, nullptr, 0);
    TEST_ADD_SYSTEM(api, "near", "near", &two[1], 1, nullptr, 0);
    TEST_ADD_SYSTEM(api, "far", "far", &two[1], 1, nullptr, 0);
}

#ifndef BIG
static void add_observers(const test_api_t *api)
{
    bridge_term_t one[1] = {{ ndirect_d, BRIDGE_READ, BRIDGE_AND, 0 }};
    bridge_term_t two[2] = {{ ndirect_a, BRIDGE_READ, BRIDGE_AND, 0 },
        { ndirect_d, BRIDGE_READ, BRIDGE_AND, 0 }};

    ndirect_on_d = TEST_OBSERVER(api, "heard_d",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, one, 1, nullptr,
        0);
    ndirect_on_both = TEST_OBSERVER(api, "heard_both",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, two, 2, nullptr,
        0);
}

#endif

#ifdef BAD
static void add_refused(const test_api_t *api)
{
    bridge_term_t two[2] = {{ ndirect_a, BRIDGE_READ, BRIDGE_AND, 0 },
        { ndirect_d, BRIDGE_READ, BRIDGE_AND, 0 }};

    TEST_ADD_SYSTEM(api, "both", "never", two, 2, nullptr, 0);
}

#endif

TEST(init)
{
    ndirect_a = TEST_COMPONENT(api, "st:A", a_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    ndirect_d = TEST_COMPONENT(api, "st:D", d_t, BRIDGE_DELETE,
        BRIDGE_DIRECT_INDEXED);
    add_changes(api);
    add_readers(api);
#ifndef BIG
    add_observers(api);
#endif
#ifdef BAD
    add_refused(api);
#endif
    return (0);
}
