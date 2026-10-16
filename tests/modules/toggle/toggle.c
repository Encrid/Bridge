/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "toggle.h"
#include "idle.h"

#ifndef BIG
static uint32_t on_t;

TEST(heard_t)
{
    const bridge_list_t *list = test_get_list(api, on_t);
    const bridge_trigger_t *trigger;

    trigger = (const bridge_trigger_t *)(api->private_base + list->at);
    if (list->count)
        say(api, "heard of T: how many, and the events of the first",
            list->count, trigger[0].events);
    return (0);
}

#endif

static void add_changes(const test_api_t *api)
{
    int none = 0;
    uint32_t made[2] = { ntoggle_a, ntoggle_t };

    TEST_ADD_SYSTEM(api, "spawn", "spawn", &none, 0, made, 2);
    TEST_ADD_SYSTEM(api, "respawn_half", "respawn_half", &none, 0,
        made, 2);
    TEST_ADD_SYSTEM(api, "attach_b", "attach_b", &none, 0,
        &ntoggle_b, 1);
    TEST_ADD_SYSTEM(api, "destroy_half", "destroy_half", &none, 0,
        nullptr, 0);
    TEST_ADD_SYSTEM(api, "value_a", "value_a", &none, 0, &ntoggle_t,
        1);
    TEST_ADD_SYSTEM(api, "find", "find", &none, 0, nullptr, 0);
}

static void add_walks(const test_api_t *api)
{
    bridge_term_t one[1] = {{ ntoggle_t, BRIDGE_READ, BRIDGE_AND, 0 }};

#ifndef BIG
    on_t = TEST_OBSERVER(api, "heard_t",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, one, 1, nullptr,
        0);
#endif
    ntoggle_reading = TEST_ADD_SYSTEM(api, "look", "look", one, 1,
        nullptr, 0);
    TEST_ADD_SYSTEM(api, "look_b", "look_b", one, 1, nullptr, 0);
    one[0].access = BRIDGE_WRITE;
    ntoggle_writing = TEST_ADD_SYSTEM(api, "flicker", "flicker", one,
        1, nullptr, 0);
    TEST_ADD_SYSTEM(api, "combine", "combine", one, 1, nullptr, 0);
}

TEST(init)
{
    ntoggle_a = TEST_COMPONENT(api, "st:A", a_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    ntoggle_b = TEST_COMPONENT(api, "st:B", b_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    ntoggle_t = TEST_COMPONENT(api, "st:T", t_t, BRIDGE_DELETE,
        BRIDGE_TOGGLE);
    add_changes(api);
    add_walks(api);
    return (0);
}
