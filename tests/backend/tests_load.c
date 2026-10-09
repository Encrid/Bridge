/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/backend/tests_load.c -- the three entries a module calls while it
** loads, and the table filled.
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

/*
** tests_fill_api
**
** Writes the eight entries and the four moves into the table a module is
** handed.
*/
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
