/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "native_inside.h"

uint64_t native_spawn(const bridge_api_t *api)
{
    uint64_t args[1] = {};

    return (native_enter(api, NATIVE_SPAWN, args));
}

void native_destroy(const bridge_api_t *api, uint64_t id)
{
    native_enter(api, NATIVE_DESTROY, &id);
}

void native_attach(const bridge_api_t *api, uint64_t id, uint32_t slot,
    uint32_t ptr)
{
    uint64_t args[3] = { id, slot, ptr };

    native_enter(api, NATIVE_ATTACH, args);
}

void native_detach(const bridge_api_t *api, uint64_t id, uint32_t slot)
{
    uint64_t args[2] = { id, slot };

    native_enter(api, NATIVE_DETACH, args);
}

uint32_t native_alloc(const bridge_api_t *api, uint32_t len)
{
    uint64_t args[1] = { len };

    return ((uint32_t)native_enter(api, NATIVE_ALLOC, args));
}
