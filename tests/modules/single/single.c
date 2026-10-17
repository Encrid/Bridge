/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "single.h"
#include "idle.h"

uint32_t nsingle_g;
uint32_t nsingle_a;
uint32_t nsingle_on_g;

static void add_changes(const test_api_t *api)
{
    int none = 0;
    uint32_t both[2];

    nsingle_g = TEST_COMPONENT(api, "st:G", g_t, BRIDGE_DELETE,
        BRIDGE_SINGLETON);
    nsingle_a = TEST_COMPONENT(api, "st:A", a_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    both[0] = nsingle_g;
    both[1] = nsingle_a;
    TEST_ADD_SYSTEM(api, "attach_a", "attach_a", &none, 0, both, 2);
    TEST_ADD_SYSTEM(api, "value_a", "value_a", &none, 0, &nsingle_g,
        1);
    TEST_ADD_SYSTEM(api, "detach_b", "detach_b", &none, 0,
        &nsingle_g, 1);
    TEST_ADD_SYSTEM(api, "flicker", "flicker", &none, 0,
        &nsingle_g, 1);
}

static void add_readers(const test_api_t *api)
{
    bridge_term_t one[1] = {{ nsingle_g, BRIDGE_READ, BRIDGE_OPTIONAL, 0 }};

    TEST_ADD_SYSTEM(api, "look", "look", one, 1, nullptr, 0);
    TEST_ADD_SYSTEM(api, "one", "one", one, 1, nullptr, 0);
    nsingle_which = nsingle_g;
    one[0].access = BRIDGE_WRITE;
    TEST_ADD_SYSTEM(api, "work", "work", one, 1, nullptr, 0);
    one[0].access = BRIDGE_READ;
    one[0].oper = BRIDGE_AND;
    nsingle_on_g = TEST_OBSERVER(api, "heard_g",
        BRIDGE_ON_ENTER | BRIDGE_ON_ATTACH | BRIDGE_ON_LEAVE, one, 1, nullptr,
        0);
#if defined(BAD) && BAD == 1
    TEST_ADD_SYSTEM(api, "by", "never", one, 1, nullptr, 0);
#endif
}

TEST(init)
{
    add_changes(api);
    add_readers(api);
    return (0);
}
