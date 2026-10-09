/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/binding/offsets_phase.c -- three entries of the ABI a module calls
** in a phase, and two reads of the private memory.
**
** abi.md, *The ABI*
*/

#include <string.h>

#include "test_offsets_acc.h"

/*
** bridge_attach
**
** attach(id, slot, ptr).
*/
void bridge_attach(uint64_t id, uint32_t slot, uint32_t ptr)
{
    const test_api_t *api = test_get_table();

    api->attach(api, id, slot, ptr);
}

/*
** bridge_detach
**
** detach(id, slot).
*/
void bridge_detach(uint64_t id, uint32_t slot)
{
    const test_api_t *api = test_get_table();

    api->detach(api, id, slot);
}

/*
** bridge_alloc
**
** alloc(len) -> offset.
*/
uint32_t bridge_alloc(uint32_t len)
{
    const test_api_t *api = test_get_table();

    return (api->alloc(api, len));
}

/*
** acc_own_32
**
** Reads 32 bits of the private memory.
*/
uint32_t acc_own_32(uint32_t at)
{
    uint32_t v;

    memcpy(&v, test_get_table()->private_base + at, sizeof(v));
    return (v);
}

/*
** acc_own_64
**
** Reads 64 bits of the private memory.
*/
uint64_t acc_own_64(uint32_t at)
{
    uint64_t v;

    memcpy(&v, test_get_table()->private_base + at, sizeof(v));
    return (v);
}
