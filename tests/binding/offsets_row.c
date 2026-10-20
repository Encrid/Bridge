/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include <string.h>

#include "test_offsets_acc.h"

static const test_api_t **table_at(void)
{
    static thread_local const test_api_t *table;

    return (&table);
}

TEST_EXPORT void test_keep_table(const test_api_t *api)
{
    *table_at() = api;
}

const test_api_t *test_get_table(void)
{
    return (*table_at());
}

void acc_trap(void)
{
    const test_api_t *api = test_get_table();

    api->trap(api);
}

uint32_t acc_resolve(uint32_t slot, uint32_t a, uint32_t b, uint32_t field,
    uint32_t width, uint32_t store)
{
    uint32_t types = BRIDGE_HEADER(types);
    uint32_t kind;
    uint32_t stride;
    uint32_t base;
    uint32_t cell;

    if (slot >= types)
        acc_trap();
    stride = acc_own_32(BRIDGE_HEADER(stride) + slot * 4u);
    if (store && !acc_own_32(BRIDGE_HEADER(write) + slot * 4u))
        acc_trap();
    if (field + width > stride)
        acc_trap();
    kind = acc_own_32(BRIDGE_HEADER(kind) + slot * 4u);
    if (kind == BRIDGE_TABLE || kind == BRIDGE_TOGGLE) {
        if (a >= BRIDGE_HEADER(tables))
            acc_trap();
        base = acc_own_32(BRIDGE_HEADER(column_base) + 4u * (a * types + slot));
        if (!base || b >= acc_own_32(BRIDGE_HEADER(rows) + a * 4u))
            acc_trap();
        return (base + b * stride + field);
    }
    if (kind == BRIDGE_SINGLETON) {
        base = acc_own_32(BRIDGE_HEADER(kept) + slot * 4u);
        if (!base)
            acc_trap();
        return (base + field);
    }
    if (a >= BRIDGE_HEADER(entities))
        acc_trap();
    base = acc_own_32(BRIDGE_HEADER(kept) + slot * 4u);
    if (kind == BRIDGE_SPARSE_SET) {
        cell = acc_peek_32(base + a * 4u);
        if (!cell)
            acc_trap();
        return (cell + field);
    }
    if (kind == BRIDGE_DIRECT_INDEXED) {
        if (!(acc_peek_64(acc_peek_32(base - 16u) + (a >> 6) * 8u)
                >> (a & 63u) & 1u))
            acc_trap();
        return (base + a * stride + field);
    }
    acc_trap();
    return (0);
}

uint32_t acc_toggle_bits(uint32_t slot, uint32_t table, uint32_t row)
{
    uint32_t types = BRIDGE_HEADER(types);
    uint32_t base;

    if (slot >= types)
        acc_trap();
    if (acc_own_32(BRIDGE_HEADER(kind) + slot * 4u) != BRIDGE_TOGGLE)
        acc_trap();
    if (table >= BRIDGE_HEADER(tables))
        acc_trap();
    base = acc_own_32(BRIDGE_HEADER(column_base) + 4u * (table * types + slot));
    if (!base || row >= acc_own_32(BRIDGE_HEADER(rows) + table * 4u))
        acc_trap();
    return (acc_peek_32(base - 16u) + (row >> 6) * 8u);
}
