/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "store.h"

#if !defined(BIG) && !defined(TABLES)
static void add_first_changes(const test_api_t *api)
{
    int none = 0;
    uint32_t both[2] = { nstore_a, nstore_b };

    TEST_ADD_SYSTEM(api, "attach_b_half", "attach_b_half", &none, 0,
        &nstore_b, 1);
    TEST_ADD_SYSTEM(api, "detach_b_half", "detach_b_half", &none, 0,
        &nstore_b, 1);
    TEST_ADD_SYSTEM(api, "detach_b_absent", "detach_b_absent", &none,
        0, &nstore_b, 1);
    TEST_ADD_SYSTEM(api, "value_a", "value_a", &none, 0, &nstore_a,
        1);
    TEST_ADD_SYSTEM(api, "flicker", "flicker", &none, 0, both, 2);
    TEST_ADD_SYSTEM(api, "destroy_half", "destroy_half", &none, 0,
        nullptr, 0);
    TEST_ADD_SYSTEM(api, "respawn_half", "respawn_half", &none, 0,
        nullptr, 0);
}

static void add_last_changes(const test_api_t *api)
{
    int none = 0;

    TEST_ADD_SYSTEM(api, "touch_dead", "touch_dead", &none, 0,
        &nstore_a, 1);
    TEST_ADD_SYSTEM(api, "recycled", "recycled", &none, 0, nullptr,
        0);
    TEST_ADD_SYSTEM(api, "attach_a_half", "attach_a_half", &none, 0,
        &nstore_a, 1);
    TEST_ADD_SYSTEM(api, "scribble", "scribble", &none, 0, nullptr,
        0);
    TEST_ADD_SYSTEM(api, "find", "find", &none, 0, nullptr, 0);
    TEST_ADD_SYSTEM(api, "value_b", "value_b", &none, 0, &nstore_b,
        1);
}

static void add_looks(const test_api_t *api)
{
    bridge_term_t one[1] = {{ nstore_a, BRIDGE_READ, BRIDGE_AND, 0 }};
    bridge_term_t two[2] = {{ nstore_a, BRIDGE_READ, BRIDGE_AND, 0 },
        { nstore_b, BRIDGE_READ, BRIDGE_NOT, 0 }};

    nstore_with_a = TEST_ADD_SYSTEM(api, "look", "look", one, 1,
        nullptr, 0);
    nstore_without_b = TEST_ADD_SYSTEM(api, "look_not", "never", two,
        2, nullptr, 0);
    two[1].oper = BRIDGE_OPTIONAL;
    nstore_maybe_b = TEST_ADD_SYSTEM(api, "look_maybe", "never", two,
        2, nullptr, 0);
    nstore_on_a = TEST_OBSERVER(api, "heard_a",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, one, 1, nullptr,
        0);
}

static void add_hearers(const test_api_t *api)
{
    bridge_term_t one[1] = {{ nstore_b, BRIDGE_READ, BRIDGE_AND, 0 }};
    bridge_term_t two[2] = {{ nstore_a, BRIDGE_READ, BRIDGE_AND, 0 },
        { nstore_b, BRIDGE_READ, BRIDGE_AND, 0 }};

    nstore_with_b = TEST_ADD_SYSTEM(api, "look_b", "look_b", one, 1,
        nullptr, 0);
    nstore_on_b = TEST_OBSERVER(api, "heard_b",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, one, 1, nullptr,
        0);
    nstore_on_both = TEST_OBSERVER(api, "heard_both",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, two, 2, nullptr,
        0);
    two[1].oper = BRIDGE_NOT;
    nstore_on_alone = TEST_OBSERVER(api, "heard_alone",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, two, 2, nullptr,
        0);
}

void nstore_add_plain(const test_api_t *api)
{
    add_first_changes(api);
    add_last_changes(api);
    add_looks(api);
    add_hearers(api);
}

#endif
