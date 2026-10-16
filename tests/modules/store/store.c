/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "store.h"
#include "measured.h"
#include "idle.h"

TEST_MEASURED TEST(spawn)
{
    uint32_t i = 0;

    while (i < COUNT) {
        nstore_ids[i] = api->spawn(api);
        i++;
    }
    return (0);
}

static void add_changes(const test_api_t *api)
{
    int none = 0;
    uint32_t both[2] = { nstore_a, nstore_b };

    TEST_ADD_SYSTEM(api, "spawn", "spawn", &none, 0, nullptr, 0);
    TEST_ADD_SYSTEM(api, "attach_a", "attach_a", &none, 0, &nstore_a,
        1);
    TEST_ADD_SYSTEM(api, "attach_b", "attach_b", &none, 0, &nstore_b,
        1);
    TEST_ADD_SYSTEM(api, "detach_b", "detach_b", &none, 0, &nstore_b,
        1);
    TEST_ADD_SYSTEM(api, "destroy", "destroy", &none, 0, nullptr, 0);
    TEST_ADD_SYSTEM(api, "attach_both", "attach_both", &none, 0,
        both, 2);
}

TEST(init)
{
    nstore_a = TEST_COMPONENT(api, "st:A", a_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    nstore_b = TEST_COMPONENT(api, "st:B", b_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    add_changes(api);
#ifdef TABLES
    nstore_add_tables(api);
#endif
#if !defined(BIG) && !defined(TABLES)
    nstore_add_plain(api);
#endif
    return (0);
}
