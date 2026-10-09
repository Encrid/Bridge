/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/binding/offsets_entries.c -- the entries of the ABI a module calls
** while it loads, and two it calls in a phase.
**
** abi.md, *The ABI*
*/

#include <string.h>

#include "test_offsets_acc.h"

/*
** bridge_declare
**
** declare(at) -> slot.
*/
uint32_t bridge_declare(uint32_t at)
{
    const test_api_t *api = test_get_table();

    return (api->declare(api, at));
}

/*
** bridge_add_system
**
** add_system(at) -> query.
*/
uint32_t bridge_add_system(uint32_t at)
{
    const test_api_t *api = test_get_table();

    return (api->add_system(api, at));
}

/*
** bridge_observe
**
** observe(at) -> observer.
*/
uint32_t bridge_observe(uint32_t at)
{
    const test_api_t *api = test_get_table();

    return (api->observe(api, at));
}

/*
** bridge_spawn
**
** spawn() -> id.
*/
uint64_t bridge_spawn(void)
{
    const test_api_t *api = test_get_table();

    return (api->spawn(api));
}

/*
** bridge_destroy
**
** destroy(id).
*/
void bridge_destroy(uint64_t id)
{
    const test_api_t *api = test_get_table();

    api->destroy(api, id);
}
