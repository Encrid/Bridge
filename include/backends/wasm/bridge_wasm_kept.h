/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_WASM_KEPT_H
    #define BRIDGE_WASM_KEPT_H

    #include "bridge_wasm_acc.h"

static inline int bridge_has(uint32_t slot, uint64_t id)
{
    return (acc_has(slot, (uint32_t)id));
}

static inline uint32_t bridge_base(uint32_t slot, uint32_t table)
{
    return (acc_own_32(BRIDGE_HEADER(column_base)
        + (table * BRIDGE_HEADER(types) + slot) * 4u));
}

#endif
