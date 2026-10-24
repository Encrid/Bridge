/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "stress.h"
#include "idle.h"

static const char *const NAMES[KINDS] = {
    SPACE ":K0", SPACE ":K1", SPACE ":K2", SPACE ":K3",
    SPACE ":K4", SPACE ":K5", SPACE ":K6", SPACE ":K7"
};
static const char *const SYSTEMS[KINDS] = {
    "see0", "see1", "see2", "see3", "see4", "see5", "see6", "see7"
};

uint32_t nstress_kinds[KINDS];
uint32_t nstress_seeing[KINDS];

static void declare_kind(const test_api_t *api, uint32_t k)
{
    bridge_term_t term[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    uint32_t len = (uint32_t)strlen(NAMES[k]);

    nstress_kinds[k] = api->declare(api, TEST_STATE(api,
        bridge_declaration_t, test_put(api, NAMES[k], len), len,
        BRIDGE_STRUCT, sizeof(v_t), alignof(v_t),
        test_put(api, v_t_desc, sizeof(v_t_desc) - 1),
        sizeof(v_t_desc) - 1, BRIDGE_DELETE, BRIDGE_TABLE));
    term[0].slot = nstress_kinds[k];
    nstress_seeing[k] = api->add_system(api, TEST_STATE(api,
        bridge_system_statement_t, test_put(api, SYSTEMS[k], 4), 4,
        test_put(api, "check", 5), 5,
        test_put(api, term, sizeof(term)), 1, 0, 0, 0, 0));
}

TEST(init)
{
    int none = 0;
    uint32_t k = 0;

    while (k < KINDS) {
        declare_kind(api, k);
        k++;
    }
    TEST_ADD_SYSTEM(api, "work", "work", &none, 0, nstress_kinds,
        KINDS, nullptr, 0);
    return (0);
}
