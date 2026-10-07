/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** backends/wasm/sdk/bridge_wasm.h -- what a wasm module written in C
** compiles against: the eight imports, the accessors of bridge_wasm.wat,
** the schema shim, and the layouts of bridge_abi.h read through them.
**
** Nothing here is generated: sizeof, offsetof and _Generic compute at a
** module's build what a generator would have had to be told. Every function
** is inline, so a module pays for what it uses and the merge leaves plain
** loads behind.
**
** wasm.md, *Memories*
** schema.md, *What a module compiles against*
*/

#ifndef BRIDGE_WASM_H
    #define BRIDGE_WASM_H

    #include "bridge_wasm_acc.h"
    #include "bridge_wasm_entity.h"
    #include "bridge_wasm_kept.h"
    #include "bridge_wasm_log.h"
    #include "bridge_wasm_observer.h"
    #include "bridge_wasm_scratch.h"
    #include "bridge_wasm_span.h"
    #include "bridge_wasm_toggle.h"

/*
** Registration, with names as literals. A type another declaration's fields
** use is declared before it, in the same namespace.
*/
    #define BRIDGE_DECLARE(kind, name, T, on_unload, storage) \
        bridge_declare(BRIDGE_STATE(bridge_declaration_t, \
            bridge_put(name, sizeof(name) - 1), sizeof(name) - 1, kind, \
            sizeof(T), alignof(T), \
            bridge_put(T##_desc, sizeof(T##_desc) - 1), sizeof(T##_desc) - 1, \
            on_unload, storage))
    #define BRIDGE_COMPONENT(name, T, on_unload, storage) \
        BRIDGE_DECLARE(BRIDGE_STRUCT, name, T, on_unload, storage)

/*
** A tag: a component with no bytes, so no type to take them from.
**
** storage.md, *A tag*
*/
    #define BRIDGE_DECLARE_TAG(name, on_unload, storage) \
        bridge_declare(BRIDGE_STATE(bridge_declaration_t, \
            bridge_put(name, sizeof(name) - 1), sizeof(name) - 1, BRIDGE_TAG, \
            0, 1, 0, 0, on_unload, storage))

/*
** A system, and an observer. `changes` is the slots of the components it
** attaches or detaches, `change_count` of them: one that does neither hands
** nullptr and 0.
*/
    #define BRIDGE_ADD_SYSTEM(name, phase, terms, count, changes, \
        change_count) \
        bridge_add_system(BRIDGE_STATE(bridge_system_statement_t, \
            bridge_put(name, sizeof(name) - 1), sizeof(name) - 1, \
            bridge_put(phase, sizeof(phase) - 1), sizeof(phase) - 1, \
            bridge_put(terms, (count) * sizeof(bridge_term_t)), count, \
            BRIDGE_PUT_CHANGES(changes, change_count), change_count))
    #define BRIDGE_OBSERVER(name, events, terms, count, changes, change_count) \
        bridge_observe(BRIDGE_STATE(bridge_observer_statement_t, \
            bridge_put(name, sizeof(name) - 1), sizeof(name) - 1, events, \
            bridge_put(terms, (count) * sizeof(bridge_term_t)), count, \
            BRIDGE_PUT_CHANGES(changes, change_count), change_count))
    #define BRIDGE_PUT_CHANGES(changes, count) \
        ((count) ? bridge_put(changes, (count) * sizeof(uint32_t)) : 0)

/*
** An attach with its value. An initial value travels with the attach, and
** is as long as the type was declared. Bridge-ASM700 takes its copy during
** the call.
**
** abi.md, *The ABI*
*/
    #define BRIDGE_ATTACH(id, slot, value) \
        bridge_attach(id, slot, bridge_lend(&(value), sizeof(value)))

/*
** A component's fields. BRIDGE_ROW is where row `row` of a column of T is,
** and carries the row < count test; `at` is what it or bridge_get_ref
** answered.
*/
    #define BRIDGE_ROW(T, base, row, count) acc_row(base, row, count, sizeof(T))
    #define BRIDGE_GET(T, field, at) \
        BRIDGE_LD_(((T *)0)->field)((at) + (uint32_t)offsetof(T, field))
    #define BRIDGE_SET(T, field, at, value) \
        BRIDGE_ST_(((T *)0)->field)((at) + (uint32_t)offsetof(T, field), \
            (value))
    #define BRIDGE_LD_(v) _Generic((v), \
        bool: acc_ld_u8, char: acc_ld_i8, signed char: acc_ld_i8, \
        unsigned char: acc_ld_u8, short: acc_ld_i16, \
        unsigned short: acc_ld_u16, int: acc_ld_32, unsigned: acc_ld_32, \
        long: acc_ld_32, unsigned long: acc_ld_32, long long: acc_ld_64, \
        unsigned long long: acc_ld_64, float: acc_ld_f32, double: acc_ld_f64)
    #define BRIDGE_ST_(v) _Generic((v), \
        bool: acc_st_8, char: acc_st_8, signed char: acc_st_8, \
        unsigned char: acc_st_8, short: acc_st_16, unsigned short: acc_st_16, \
        int: acc_st_32, unsigned: acc_st_32, long: acc_st_32, \
        unsigned long: acc_st_32, long long: acc_st_64, \
        unsigned long long: acc_st_64, float: acc_st_f32, double: acc_st_f64)

#endif
