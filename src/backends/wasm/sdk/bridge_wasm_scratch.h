/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_WASM_SCRATCH_H
    #define BRIDGE_WASM_SCRATCH_H

    #include "bridge_wasm_acc.h"

    #define BRIDGE_STATE(T, ...) bridge_put(&(T){ __VA_ARGS__ }, sizeof(T))

static inline uint32_t bridge_take_scratch(uint32_t len)
{
    uint32_t used = (BRIDGE_HEADER(scratch_used) + 7u) & ~7u;
    uint32_t room = BRIDGE_HEADER(scratch_len);

    if (used > room || len > room - used)
        __builtin_trap();
    acc_own_st(offsetof(bridge_header_t, scratch_used), used + len);
    return (BRIDGE_HEADER(scratch) + used);
}

static inline uint32_t bridge_put(const void *bytes, uint32_t len)
{
    uint32_t at = bridge_take_scratch(len);

    acc_to_private(at, bytes, len);
    return (at);
}

static inline uint32_t bridge_lend(const void *bytes, uint32_t len)
{
    uint32_t used = (BRIDGE_HEADER(scratch_used) + 7u) & ~7u;
    uint32_t room = BRIDGE_HEADER(scratch_len);

    if (used > room || len > room - used)
        __builtin_trap();
    acc_to_private(BRIDGE_HEADER(scratch) + used, bytes, len);
    return (BRIDGE_HEADER(scratch) + used);
}

#endif
