/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** backends/native/native_phase.c -- the entries a module calls in a phase,
** as the table it is handed has them.
*/

#include "native_inside.h"

/*
** native_spawn
**
** spawn() -> id.
*/
uint64_t native_spawn(const bridge_api_t *api)
{
    uint64_t args[1] = {};

    return (native_enter(api, NATIVE_SPAWN, args));
}

/*
** native_destroy
**
** destroy(id).
*/
void native_destroy(const bridge_api_t *api, uint64_t id)
{
    native_enter(api, NATIVE_DESTROY, &id);
}

/*
** native_attach
**
** attach(id, slot, ptr).
*/
void native_attach(const bridge_api_t *api, uint64_t id, uint32_t slot,
    uint32_t ptr)
{
    uint64_t args[3] = { id, slot, ptr };

    native_enter(api, NATIVE_ATTACH, args);
}

/*
** native_detach
**
** detach(id, slot).
*/
void native_detach(const bridge_api_t *api, uint64_t id, uint32_t slot)
{
    uint64_t args[2] = { id, slot };

    native_enter(api, NATIVE_DETACH, args);
}

/*
** native_alloc
**
** alloc(len) -> offset.
*/
uint32_t native_alloc(const bridge_api_t *api, uint32_t len)
{
    uint64_t args[1] = { len };

    return ((uint32_t)native_enter(api, NATIVE_ALLOC, args));
}
