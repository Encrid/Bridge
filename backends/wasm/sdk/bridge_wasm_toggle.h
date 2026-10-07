/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** backends/wasm/sdk/bridge_wasm_toggle.h -- a toggled component: a bit a
** row.
**
** storage.md, *A toggle*
*/

#ifndef BRIDGE_WASM_TOGGLE_H
    #define BRIDGE_WASM_TOGGLE_H

    #include "bridge_wasm_acc.h"

/*
** bridge_get_bits
**
** Answers where the bits of the toggled column at `base` are: row r is bit
** r & 63 of word r >> 6.
**
** storage.md, *A toggle*
*/
static inline uint32_t bridge_get_bits(uint32_t base)
{
    return (acc_ld_32(base - 16u));
}

/*
** bridge_is_on
**
** Answers whether row `row` of `count` is on.
**
** storage.md, *A toggle*
*/
static inline bool bridge_is_on(uint32_t bits, uint32_t row, uint32_t count)
{
    uint32_t at = acc_row(bits, row >> 6, (count + 63u) >> 6, 8u);

    return (acc_ld_64(at) >> (row & 63u) & 1u);
}

/*
** bridge_turn
**
** Turns a row on or off, for a system that stated the term BRIDGE_WRITE.
**
** storage.md, *A toggle*
*/
static inline void bridge_turn(uint32_t bits, uint32_t row, uint32_t count,
    bool on)
{
    uint32_t at = acc_row(bits, row >> 6, (count + 63u) >> 6, 8u);
    uint64_t word = (uint64_t)acc_ld_64(at);
    uint64_t bit = (uint64_t)1 << (row & 63u);

    acc_st_64(at, on ? word | bit : word & ~bit);
}

#endif
