/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_WASM_KEPT_H
    #define BRIDGE_WASM_KEPT_H

    #include "bridge_wasm_acc.h"

static inline uint32_t bridge_get_sparse_index(uint32_t slot)
{
    return (acc_own_32(BRIDGE_HEADER(kept) + 4u * slot));
}

static inline uint32_t bridge_get_sparse_value(uint32_t index, uint64_t id)
{
    uint32_t e = (uint32_t)id;

    return (acc_ld_32(acc_row(index, e, BRIDGE_HEADER(entities), 4u)));
}

static inline uint32_t bridge_get_single(uint32_t slot)
{
    return (acc_own_32(BRIDGE_HEADER(kept) + 4u * slot));
}

static inline uint32_t bridge_get_cells(uint32_t slot)
{
    return (acc_own_32(BRIDGE_HEADER(kept) + 4u * slot));
}

static inline uint32_t bridge_get_cell(uint32_t cells, uint64_t id,
    uint32_t stride)
{
    uint32_t entities = BRIDGE_HEADER(entities);
    uint32_t words = (entities + 63u) >> 6;
    uint32_t e = (uint32_t)id;
    uint32_t word = acc_row(acc_ld_32(cells - 16u), e >> 6, words, 8u);

    if (!(acc_ld_64(word) >> (e & 63u) & 1u))
        return (0);
    return (acc_row(cells, e, entities, stride));
}

#endif
