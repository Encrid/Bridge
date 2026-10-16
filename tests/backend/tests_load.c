/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "tests_inside.h"

static uint32_t declare(const test_api_t *api, uint32_t at)
{
    uint64_t args[1] = { at };

    return ((uint32_t)tests_enter(api, TESTS_DECLARE, args));
}

static uint32_t add_system(const test_api_t *api, uint32_t at)
{
    uint64_t args[1] = { at };

    return ((uint32_t)tests_enter(api, TESTS_ADD_SYSTEM, args));
}

static uint32_t observe(const test_api_t *api, uint32_t at)
{
    uint64_t args[1] = { at };

    return ((uint32_t)tests_enter(api, TESTS_OBSERVE, args));
}

void tests_fill_api(test_api_t *api)
{
    api->declare = declare;
    api->add_system = add_system;
    api->observe = observe;
    api->spawn = tests_spawn;
    api->destroy = tests_destroy;
    api->attach = tests_attach;
    api->detach = tests_detach;
    api->alloc = tests_alloc;
    api->trap = tests_trap;
    api->spend = tests_spend;
    api->wait = tests_wait;
    api->grow = tests_grow;
}
