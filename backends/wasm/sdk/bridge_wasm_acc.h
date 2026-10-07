/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** backends/wasm/sdk/bridge_wasm_acc.h -- what a wasm module imports: the
** eight entries of the ABI, and the accessors of bridge_wasm.wat, through
** which it reaches the two memories.
**
** What is compiled with this file is a work based on Bridge and is under
** the same licence. Other terms: docs/licence.md.
**
** abi.md, *The ABI*
** wasm.md, *A compiled module reaches them through accessors*
*/

#ifndef BRIDGE_WASM_ACC_H
    #define BRIDGE_WASM_ACC_H

    #include <stddef.h>

    #include "bridge_abi.h"
    #include "bridge_schema.h"

/*
** An attribute takes a literal, so the namespace is spelled here: it is
** BRIDGE_NAMESPACE.
**
** wasm.md, *A compiled module reaches them through accessors*
*/
    #define BRIDGE_IMPORT(name) \
        [[clang::import_module("bridge"), clang::import_name(#name)]]
    #define BRIDGE_ACC(name) \
        [[clang::import_module("acc"), clang::import_name(#name)]]
    #define BRIDGE_EXPORT(name) [[clang::export_name(#name)]]

/*
** A member of the header of the module's private memory.
**
** memory.md, *Where the regions live*
*/
    #define BRIDGE_HEADER(field) acc_own_32(offsetof(bridge_header_t, field))

/*
** The ABI. Every pointer is an offset into the scratch arena.
**
** abi.md, *The ABI*
*/
BRIDGE_IMPORT(declare) uint32_t bridge_declare(uint32_t at);
BRIDGE_IMPORT(add_system) uint32_t bridge_add_system(uint32_t at);
BRIDGE_IMPORT(observe) uint32_t bridge_observe(uint32_t at);
BRIDGE_IMPORT(spawn) uint64_t bridge_spawn(void);
BRIDGE_IMPORT(destroy) void bridge_destroy(uint64_t id);
BRIDGE_IMPORT(attach) void bridge_attach(uint64_t id, uint32_t slot,
    uint32_t ptr);
BRIDGE_IMPORT(detach) void bridge_detach(uint64_t id, uint32_t slot);
BRIDGE_IMPORT(alloc) uint32_t bridge_alloc(uint32_t len);

/*
** Where a row is. Whether it is tested to be one of the span's rows, each
** time, is stated by whoever builds the module, and the accessor that is
** imported under the one name is the one that was stated.
**
** validation.md, *The row test*
*/
    #if defined(BRIDGE_ROWS_CHECKED) == defined(BRIDGE_ROWS_UNCHECKED)
        #error "state BRIDGE_ROWS_CHECKED or BRIDGE_ROWS_UNCHECKED: one of them"
    #endif
    #ifdef BRIDGE_ROWS_CHECKED
BRIDGE_ACC(acc_row) uint32_t acc_row(uint32_t base, uint32_t row,
    uint32_t count, uint32_t stride);
    #else
BRIDGE_ACC(acc_row_unchecked) uint32_t acc_row(uint32_t base, uint32_t row,
    uint32_t count, uint32_t stride);
    #endif

/*
** Loads from the shared memory and stores to it, by the kind of the value.
**
** wasm.md, *A compiled module reaches them through accessors*
*/
BRIDGE_ACC(acc_ld_i8) int32_t acc_ld_i8(uint32_t at);
BRIDGE_ACC(acc_ld_u8) uint32_t acc_ld_u8(uint32_t at);
BRIDGE_ACC(acc_ld_i16) int32_t acc_ld_i16(uint32_t at);
BRIDGE_ACC(acc_ld_u16) uint32_t acc_ld_u16(uint32_t at);
BRIDGE_ACC(acc_ld_32) uint32_t acc_ld_32(uint32_t at);
BRIDGE_ACC(acc_ld_64) uint64_t acc_ld_64(uint32_t at);
BRIDGE_ACC(acc_ld_f32) float acc_ld_f32(uint32_t at);
BRIDGE_ACC(acc_ld_f64) double acc_ld_f64(uint32_t at);
BRIDGE_ACC(acc_st_8) void acc_st_8(uint32_t at, uint32_t v);
BRIDGE_ACC(acc_st_16) void acc_st_16(uint32_t at, uint32_t v);
BRIDGE_ACC(acc_st_32) void acc_st_32(uint32_t at, uint32_t v);
BRIDGE_ACC(acc_st_64) void acc_st_64(uint32_t at, uint64_t v);
BRIDGE_ACC(acc_st_f32) void acc_st_f32(uint32_t at, float v);
BRIDGE_ACC(acc_st_f64) void acc_st_f64(uint32_t at, double v);

/*
** Loads from the module's private memory and a store to it, and the copies
** between the module's own memory and the two.
**
** wasm.md, *A compiled module reaches them through accessors*
*/
BRIDGE_ACC(acc_own_32) uint32_t acc_own_32(uint32_t at);
BRIDGE_ACC(acc_own_64) uint64_t acc_own_64(uint32_t at);
BRIDGE_ACC(acc_own_st) void acc_own_st(uint32_t at, uint32_t v);
BRIDGE_ACC(acc_to_private) void acc_to_private(uint32_t to, const void *from,
    uint32_t len);
BRIDGE_ACC(acc_to_shared) void acc_to_shared(uint32_t to, const void *from,
    uint32_t len);
BRIDGE_ACC(acc_from_shared) void acc_from_shared(void *to, uint32_t from,
    uint32_t len);

#endif
