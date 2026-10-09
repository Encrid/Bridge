/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/backend/tests_phase.c -- the five entries a module calls in a
** phase.
**
** abi.md, *The ABI*
*/

#include "tests_inside.h"

/*
** tests_spawn
**
** spawn() -> id.
*/
uint64_t tests_spawn(const test_api_t *api)
{
    uint64_t args[1] = {};

    return (tests_enter(api, TESTS_SPAWN, args));
}

/*
** tests_destroy
**
** destroy(id).
*/
void tests_destroy(const test_api_t *api, uint64_t id)
{
    tests_enter(api, TESTS_DESTROY, &id);
}

/*
** tests_attach
**
** attach(id, slot, ptr).
*/
void tests_attach(const test_api_t *api, uint64_t id, uint32_t slot,
    uint32_t ptr)
{
    uint64_t args[3] = { id, slot, ptr };

    tests_enter(api, TESTS_ATTACH, args);
}

/*
** tests_detach
**
** detach(id, slot).
*/
void tests_detach(const test_api_t *api, uint64_t id, uint32_t slot)
{
    uint64_t args[2] = { id, slot };

    tests_enter(api, TESTS_DETACH, args);
}

/*
** tests_alloc
**
** alloc(len) -> offset.
*/
uint32_t tests_alloc(const test_api_t *api, uint32_t len)
{
    uint64_t args[1] = { len };

    return ((uint32_t)tests_enter(api, TESTS_ALLOC, args));
}
