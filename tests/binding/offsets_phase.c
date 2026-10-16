/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "test_offsets_acc.h"

void bridge_attach(uint64_t id, uint32_t slot, uint32_t ptr)
{
    const test_api_t *api = test_get_table();

    api->attach(api, id, slot, ptr);
}

void bridge_detach(uint64_t id, uint32_t slot)
{
    const test_api_t *api = test_get_table();

    api->detach(api, id, slot);
}

uint32_t bridge_alloc(uint32_t len)
{
    const test_api_t *api = test_get_table();

    return (api->alloc(api, len));
}

uint32_t acc_own_32(uint32_t at)
{
    uint32_t v;

    memcpy(&v, test_get_table()->private_base + at, sizeof(v));
    return (v);
}

uint64_t acc_own_64(uint32_t at)
{
    uint64_t v;

    memcpy(&v, test_get_table()->private_base + at, sizeof(v));
    return (v);
}
