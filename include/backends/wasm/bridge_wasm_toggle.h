/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_WASM_TOGGLE_H
    #define BRIDGE_WASM_TOGGLE_H

    #include "bridge_wasm_acc.h"

static inline int bridge_is_on(uint32_t slot, uint32_t table, uint32_t row)
{
    return (acc_toggle(slot, table, row));
}

static inline void bridge_turn(uint32_t slot, uint32_t table, uint32_t row,
    int on)
{
    acc_toggle_set(slot, table, row, on);
}

#endif
