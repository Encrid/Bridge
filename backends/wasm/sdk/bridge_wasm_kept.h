/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** backends/wasm/sdk/bridge_wasm_kept.h -- what is not kept in a table, and
** is reached from a row or by an id: a sparse set, a singleton, a
** direct-indexed component.
**
** storage.md, *A sparse set*
** storage.md, *A singleton*
** storage.md, *Direct-indexed*
*/

#ifndef BRIDGE_WASM_KEPT_H
    #define BRIDGE_WASM_KEPT_H

    #include "bridge_wasm_acc.h"

/*
** bridge_get_sparse_index
**
** Answers where the sparse set that keeps component `slot` has its index: a
** cell for each entity.
**
** storage.md, *A sparse set*
*/
static inline uint32_t bridge_get_sparse_index(uint32_t slot)
{
    return (acc_own_32(BRIDGE_HEADER(kept) + 4u * slot));
}

/*
** bridge_get_sparse_value
**
** Answers where entity `id` has its value in the set of that index, from
** the shared base: zero if it has none.
**
** storage.md, *A sparse set*
*/
static inline uint32_t bridge_get_sparse_value(uint32_t index, uint64_t id)
{
    uint32_t e = (uint32_t)id;

    return (acc_ld_32(acc_row(index, e, BRIDGE_HEADER(entities), 4u)));
}

/*
** bridge_get_single
**
** Answers where the singleton `slot` has its value, from the shared base:
** zero while it has none.
**
** storage.md, *A singleton*
*/
static inline uint32_t bridge_get_single(uint32_t slot)
{
    return (acc_own_32(BRIDGE_HEADER(kept) + 4u * slot));
}

/*
** bridge_get_cells
**
** Answers where the cells of the direct-indexed component `slot` begin.
**
** storage.md, *Direct-indexed*
*/
static inline uint32_t bridge_get_cells(uint32_t slot)
{
    return (acc_own_32(BRIDGE_HEADER(kept) + 4u * slot));
}

/*
** bridge_get_cell
**
** Answers where entity `id` has its value of `stride` bytes among those
** cells, or zero: it was given none.
**
** storage.md, *Direct-indexed*
*/
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
