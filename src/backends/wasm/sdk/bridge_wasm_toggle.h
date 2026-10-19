/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_WASM_TOGGLE_H
    #define BRIDGE_WASM_TOGGLE_H

    #include "bridge_wasm_acc.h"

static inline uint32_t bridge_get_bits(uint32_t base)
{
    return (acc_ld_32(base - 16u));
}

static inline bool bridge_is_on(uint32_t bits, uint32_t row, uint32_t count)
{
    uint32_t at = acc_row(bits, row >> 6, (count + 63u) >> 6, 8u);

    return (acc_ld_64(at) >> (row & 63u) & 1u);
}

static inline void bridge_turn(uint32_t bits, uint32_t row, uint32_t count,
    bool on)
{
    uint32_t at = acc_row(bits, row >> 6, (count + 63u) >> 6, 8u);
    uint64_t word = (uint64_t)acc_ld_64(at);
    uint64_t bit = (uint64_t)1 << (row & 63u);

    acc_st_64(at, on ? word | bit : word & ~bit);
}

#endif
