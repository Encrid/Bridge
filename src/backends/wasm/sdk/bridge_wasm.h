/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
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

    #define BRIDGE_DECLARE(kind, name, T, on_unload, storage) \
        bridge_declare(BRIDGE_STATE(bridge_declaration_t, \
            bridge_put(name, sizeof(name) - 1), sizeof(name) - 1, kind, \
            sizeof(T), alignof(T), \
            bridge_put(T##_desc, sizeof(T##_desc) - 1), sizeof(T##_desc) - 1, \
            on_unload, storage))
    #define BRIDGE_COMPONENT(name, T, on_unload, storage) \
        BRIDGE_DECLARE(BRIDGE_STRUCT, name, T, on_unload, storage)

    #define BRIDGE_DECLARE_TAG(name, on_unload, storage) \
        bridge_declare(BRIDGE_STATE(bridge_declaration_t, \
            bridge_put(name, sizeof(name) - 1), sizeof(name) - 1, BRIDGE_TAG, \
            0, 1, 0, 0, on_unload, storage))

    #define BRIDGE_ADD_SYSTEM(name, phase, terms, count, changes, \
        change_count, before, before_count) \
        bridge_add_system(BRIDGE_STATE(bridge_system_statement_t, \
            bridge_put(name, sizeof(name) - 1), sizeof(name) - 1, \
            bridge_put(phase, sizeof(phase) - 1), sizeof(phase) - 1, \
            bridge_put(terms, (count) * sizeof(bridge_term_t)), count, \
            BRIDGE_PUT_SLOTS(changes, change_count), change_count, \
            BRIDGE_PUT_SLOTS(before, before_count), before_count))
    #define BRIDGE_OBSERVER(name, events, terms, count, changes, \
        change_count, before, before_count) \
        bridge_observe(BRIDGE_STATE(bridge_observer_statement_t, \
            bridge_put(name, sizeof(name) - 1), sizeof(name) - 1, events, \
            bridge_put(terms, (count) * sizeof(bridge_term_t)), count, \
            BRIDGE_PUT_SLOTS(changes, change_count), change_count, \
            BRIDGE_PUT_SLOTS(before, before_count), before_count))
    #define BRIDGE_PUT_SLOTS(slots, count) \
        ((count) ? bridge_put(slots, (count) * sizeof(uint32_t)) : 0)

    #define BRIDGE_ATTACH(id, slot, value) \
        bridge_attach(id, slot, bridge_lend(&(value), sizeof(value)))

    #define BRIDGE_AT(a, b) (((uint64_t)(uint32_t)(a) << 32) | (uint32_t)(b))
    #define BRIDGE_GET(T, field, slot, a, b) \
        BRIDGE_LD_(((T *)0)->field)(slot, BRIDGE_AT(a, b), \
            (uint32_t)offsetof(T, field))
    #define BRIDGE_SET(T, field, slot, a, b, value) \
        BRIDGE_ST_(((T *)0)->field)(slot, BRIDGE_AT(a, b), \
            (uint32_t)offsetof(T, field), (value))
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
