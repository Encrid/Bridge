/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_OFFSETS_H
    #define TEST_OFFSETS_H

    #include <limits.h>

    #include "test_offsets_acc.h"
    #include "test_offsets_entity.h"
    #include "test_offsets_kept.h"
    #include "test_offsets_log.h"
    #include "test_offsets_observer.h"
    #include "test_offsets_scratch.h"
    #include "test_offsets_span.h"
    #include "test_offsets_toggle.h"

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

    #define BRIDGE_ATTACH(id, slot, value) \
        bridge_attach(id, slot, bridge_lend(&(value), sizeof(value)))

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
        long: BRIDGE_LD_LONG_, unsigned long: BRIDGE_LD_LONG_, \
        long long: acc_ld_64, unsigned long long: acc_ld_64, \
        float: acc_ld_f32, double: acc_ld_f64)
    #define BRIDGE_ST_(v) _Generic((v), \
        bool: acc_st_8, char: acc_st_8, signed char: acc_st_8, \
        unsigned char: acc_st_8, short: acc_st_16, unsigned short: acc_st_16, \
        int: acc_st_32, unsigned: acc_st_32, long: BRIDGE_ST_LONG_, \
        unsigned long: BRIDGE_ST_LONG_, long long: acc_st_64, \
        unsigned long long: acc_st_64, float: acc_st_f32, double: acc_st_f64)

    #if ULONG_MAX > 0xffffffffu
        #define BRIDGE_LD_LONG_ acc_ld_64
        #define BRIDGE_ST_LONG_ acc_st_64
    #else
        #define BRIDGE_LD_LONG_ acc_ld_32
        #define BRIDGE_ST_LONG_ acc_st_32
    #endif

#endif
