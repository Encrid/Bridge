/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef BRIDGE_WASM_ACC_H
    #define BRIDGE_WASM_ACC_H

    #include <stddef.h>

    #include "bridge_abi.h"
    #include "bridge_schema.h"

    #define BRIDGE_IMPORT(name) \
        [[clang::import_module("bridge"), clang::import_name(#name)]]
    #define BRIDGE_ACC(name) \
        [[clang::import_module("acc"), clang::import_name(#name)]]
    #define BRIDGE_EXPORT(name) [[clang::export_name(#name)]]

    #define BRIDGE_HEADER(field) acc_own_32(offsetof(bridge_header_t, field))

BRIDGE_IMPORT(declare) uint32_t bridge_declare(uint32_t at);
BRIDGE_IMPORT(add_system) uint32_t bridge_add_system(uint32_t at);
BRIDGE_IMPORT(observe) uint32_t bridge_observe(uint32_t at);
BRIDGE_IMPORT(spawn) uint64_t bridge_spawn(void);
BRIDGE_IMPORT(destroy) void bridge_destroy(uint64_t id);
BRIDGE_IMPORT(attach) void bridge_attach(uint64_t id, uint32_t slot,
    uint32_t ptr);
BRIDGE_IMPORT(detach) void bridge_detach(uint64_t id, uint32_t slot);
BRIDGE_IMPORT(alloc) uint32_t bridge_alloc(uint32_t len);

BRIDGE_ACC(acc_ld_i8) int32_t acc_ld_i8(uint32_t slot, uint64_t ab,
    uint32_t field);
BRIDGE_ACC(acc_ld_u8) uint32_t acc_ld_u8(uint32_t slot, uint64_t ab,
    uint32_t field);
BRIDGE_ACC(acc_ld_i16) int32_t acc_ld_i16(uint32_t slot, uint64_t ab,
    uint32_t field);
BRIDGE_ACC(acc_ld_u16) uint32_t acc_ld_u16(uint32_t slot, uint64_t ab,
    uint32_t field);
BRIDGE_ACC(acc_ld_32) uint32_t acc_ld_32(uint32_t slot, uint64_t ab,
    uint32_t field);
BRIDGE_ACC(acc_ld_64) uint64_t acc_ld_64(uint32_t slot, uint64_t ab,
    uint32_t field);
BRIDGE_ACC(acc_ld_f32) float acc_ld_f32(uint32_t slot, uint64_t ab,
    uint32_t field);
BRIDGE_ACC(acc_ld_f64) double acc_ld_f64(uint32_t slot, uint64_t ab,
    uint32_t field);
BRIDGE_ACC(acc_st_8) void acc_st_8(uint32_t slot, uint64_t ab, uint32_t field,
    uint32_t v);
BRIDGE_ACC(acc_st_16) void acc_st_16(uint32_t slot, uint64_t ab, uint32_t field,
    uint32_t v);
BRIDGE_ACC(acc_st_32) void acc_st_32(uint32_t slot, uint64_t ab, uint32_t field,
    uint32_t v);
BRIDGE_ACC(acc_st_64) void acc_st_64(uint32_t slot, uint64_t ab, uint32_t field,
    uint64_t v);
BRIDGE_ACC(acc_st_f32) void acc_st_f32(uint32_t slot, uint64_t ab,
    uint32_t field, float v);
BRIDGE_ACC(acc_st_f64) void acc_st_f64(uint32_t slot, uint64_t ab,
    uint32_t field, double v);

BRIDGE_ACC(acc_entity) uint64_t acc_entity(uint64_t id);
BRIDGE_ACC(acc_who) uint64_t acc_who(uint32_t table, uint32_t row);
BRIDGE_ACC(acc_has) uint32_t acc_has(uint32_t slot, uint32_t id);
BRIDGE_ACC(acc_toggle) uint32_t acc_toggle(uint32_t slot, uint32_t table,
    uint32_t row);
BRIDGE_ACC(acc_toggle_set) void acc_toggle_set(uint32_t slot, uint32_t table,
    uint32_t row, uint32_t on);

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
