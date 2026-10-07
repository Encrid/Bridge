/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** backends/wasm/sdk/bridge_wasm_scratch.h -- the scratch arena: where a
** module puts what an entry of the ABI is handed.
**
** What is compiled with this file is a work based on Bridge and is under
** the same licence. Other terms: docs/licence.md.
**
** wasm.md, *Memories*
*/

#ifndef BRIDGE_WASM_SCRATCH_H
    #define BRIDGE_WASM_SCRATCH_H

    #include "bridge_wasm_acc.h"

/*
** A struct of type T with these members, put in the scratch arena: where it
** is.
**
** memory.md, *Where the regions live*
*/
    #define BRIDGE_STATE(T, ...) bridge_put(&(T){ __VA_ARGS__ }, sizeof(T))

/*
** bridge_take_scratch
**
** Answers `len` bytes of the scratch arena, good until the next barrier. A
** module that asks for more than it was given stops there.
**
** memory.md, *Where the regions live*
*/
static inline uint32_t bridge_take_scratch(uint32_t len)
{
    uint32_t used = (BRIDGE_HEADER(scratch_used) + 7u) & ~7u;
    uint32_t room = BRIDGE_HEADER(scratch_len);

    if (used > room || len > room - used)
        __builtin_trap();
    acc_own_st(offsetof(bridge_header_t, scratch_used), used + len);
    return (BRIDGE_HEADER(scratch) + used);
}

/*
** bridge_put
**
** Copies `len` bytes into the scratch arena, and answers where they are.
**
** memory.md, *Where the regions live*
*/
static inline uint32_t bridge_put(const void *bytes, uint32_t len)
{
    uint32_t at = bridge_take_scratch(len);

    acc_to_private(at, bytes, len);
    return (at);
}

/*
** bridge_lend
**
** Copies `len` bytes above what the scratch arena has in use, and does not
** count them as used. For a value an entry copies during the call, as
** attach does, so that a module's scratch holds one such value at a time
** and not a phase's worth.
**
** memory.md, *Where the regions live*
*/
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
