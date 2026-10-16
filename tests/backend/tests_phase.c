/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "tests_inside.h"

uint64_t tests_spawn(const test_api_t *api)
{
    uint64_t args[1] = {};

    return (tests_enter(api, TESTS_SPAWN, args));
}

void tests_destroy(const test_api_t *api, uint64_t id)
{
    tests_enter(api, TESTS_DESTROY, &id);
}

void tests_attach(const test_api_t *api, uint64_t id, uint32_t slot,
    uint32_t ptr)
{
    uint64_t args[3] = { id, slot, ptr };

    tests_enter(api, TESTS_ATTACH, args);
}

void tests_detach(const test_api_t *api, uint64_t id, uint32_t slot)
{
    uint64_t args[2] = { id, slot };

    tests_enter(api, TESTS_DETACH, args);
}

uint32_t tests_alloc(const test_api_t *api, uint32_t len)
{
    uint64_t args[1] = { len };

    return ((uint32_t)tests_enter(api, TESTS_ALLOC, args));
}
