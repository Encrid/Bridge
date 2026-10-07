/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** backends/native/native_load.c -- the entries a module calls at load, and
** the table it is handed.
*/

#include "native_inside.h"

static uint32_t declare(const bridge_api_t *api, uint32_t at)
{
    uint64_t args[1] = { at };

    return ((uint32_t)native_enter(api, NATIVE_DECLARE, args));
}

static uint32_t add_system(const bridge_api_t *api, uint32_t at)
{
    uint64_t args[1] = { at };

    return ((uint32_t)native_enter(api, NATIVE_ADD_SYSTEM, args));
}

static uint32_t observe(const bridge_api_t *api, uint32_t at)
{
    uint64_t args[1] = { at };

    return ((uint32_t)native_enter(api, NATIVE_OBSERVE, args));
}

/*
** native_fill_api
**
** Writes the eight entries into the table a module is handed.
*/
void native_fill_api(bridge_api_t *api)
{
    api->declare = declare;
    api->add_system = add_system;
    api->observe = observe;
    api->spawn = native_spawn;
    api->destroy = native_destroy;
    api->attach = native_attach;
    api->detach = native_detach;
    api->alloc = native_alloc;
}
