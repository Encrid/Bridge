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

uint32_t acc_get_own_32(uint32_t at)
{
    uint32_t v;

    memcpy(&v, test_get_table()->private_base + at, sizeof(v));
    return (v);
}

uint64_t acc_get_own_64(uint32_t at)
{
    uint64_t v;

    memcpy(&v, test_get_table()->private_base + at, sizeof(v));
    return (v);
}

uint32_t acc_get_shared_32(uint32_t at)
{
    uint32_t v;

    memcpy(&v, test_get_table()->shared_base + at, sizeof(v));
    return (v);
}

uint64_t acc_get_shared_64(uint32_t at)
{
    uint64_t v;

    memcpy(&v, test_get_table()->shared_base + at, sizeof(v));
    return (v);
}

uint64_t acc_get_location(uint64_t id)
{
    uint32_t index = (uint32_t)id;
    uint32_t rec;

    if (index >= BRIDGE_HEADER(entities))
        return (UINT64_MAX);
    rec = BRIDGE_HEADER(entity_index)
        + index * (uint32_t)sizeof(bridge_entity_t);
    if (acc_get_shared_32(rec + 8u) != (uint32_t)(id >> 32))
        return (UINT64_MAX);
    return ((uint64_t)acc_get_shared_32(rec) << 32
        | acc_get_shared_32(rec + 4u));
}

uint32_t acc_get_rows(uint32_t table)
{
    if (table >= BRIDGE_HEADER(tables))
        acc_trap();
    return (acc_get_shared_32(BRIDGE_HEADER(rows) + table * 4u));
}

uint64_t acc_get_entity(uint32_t table, uint32_t row)
{
    if (row >= acc_get_rows(table))
        acc_trap();
    return (acc_get_shared_64(
        acc_get_shared_32(BRIDGE_HEADER(blocks) + table * 4u) + row * 8u));
}

uint32_t acc_has(uint32_t slot, uint32_t id)
{
    uint32_t kind;
    uint32_t base;

    if (slot >= BRIDGE_HEADER(types) || id >= BRIDGE_HEADER(entities))
        return (0);
    kind = acc_get_own_32(BRIDGE_HEADER(kind) + slot * 4u);
    base = acc_get_own_32(BRIDGE_HEADER(kept) + slot * 4u);
    if (kind == BRIDGE_SPARSE_SET)
        return (acc_get_shared_32(base + id * 4u) != 0);
    if (kind == BRIDGE_DIRECT_INDEXED)
        return ((uint32_t)(acc_get_shared_64(acc_get_shared_32(base - 16u)
            + (id >> 6) * 8u) >> (id & 63u)) & 1u);
    return (0);
}
