/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_NLAYOUT_H
    #define TEST_NLAYOUT_H

    #include "test_module.h"

    #if V == 9
        #define SWITCH_VALUES { OFF, AUTO, ON }
    #elif V == 10
        #define SWITCH_VALUES { OFF, ON, AUTO, MAX }
    #elif V == 12
        #define SWITCH_VALUES { OFF, ON }
    #else
        #define SWITCH_VALUES { OFF, ON, AUTO }
    #endif

    #if V == 14
BRIDGE_SCHEMA_ENUM_TYPE(switch_t, int16_t, SWITCH_VALUES);
BRIDGE_SCHEMA_ENUM_DESC(switch_t, int16_t, SWITCH_VALUES);
    #else
BRIDGE_SCHEMA_ENUM_TYPE(switch_t, int8_t, SWITCH_VALUES);
BRIDGE_SCHEMA_ENUM_DESC(switch_t, int8_t, SWITCH_VALUES);
    #endif

    #if V == 8
        #define VEC_FIELDS { uint16_t y; uint16_t x; uint16_t z; }
    #elif V == 11
        #define VEC_FIELDS { uint16_t x; }
    #elif V == 17
        #define VEC_FIELDS { uint16_t x; uint16_t y; uint16_t w; }
    #else
        #define VEC_FIELDS { uint16_t x; uint16_t y; }
    #endif

BRIDGE_SCHEMA_STRUCT_TYPE(vec_t, VEC_FIELDS);
    #if V == 16
        #define OTHER_FIELDS { uint16_t x; uint16_t y; }
BRIDGE_SCHEMA_STRUCT_TYPE(other_t, OTHER_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(other_t, OTHER_FIELDS);
    #endif
    #if V != 13
BRIDGE_SCHEMA_STRUCT_DESC(vec_t, VEC_FIELDS);
    #endif

    #if V == 1 || V == 20 || V == 21
        #define X_FIELDS { uint32_t a; uint16_t b; uint8_t arr[4]; \
            vec_t at; switch_t mode; uint32_t added; }
    #elif V == 15
        #define X_FIELDS { uint32_t a2; uint16_t b2; uint8_t arr2[4]; \
            vec_t at2; switch_t mode2; }
    #elif V == 16
        #define X_FIELDS { uint32_t a; uint16_t b; uint8_t arr[4]; \
            other_t at; switch_t mode; }
    #elif V == 17
        #define X_FIELDS { uint32_t a; uint16_t b; uint8_t arr[4]; \
            vec_t at[2]; switch_t mode; }
    #elif V == 18
        #define X_FIELDS { uint32_t a; uint16_t b; uint8_t arr[4]; \
            vec_t at; switch_t mode; \
            uint8_t removed_field_1_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx; \
            uint8_t removed_field_2_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx; \
            uint8_t removed_field_3_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx; \
            uint8_t removed_field_4_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx; \
            uint8_t removed_field_5_xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx; }
    #elif V == 2
        #define X_FIELDS { switch_t mode; vec_t at; uint8_t arr[4]; \
            uint16_t b; uint32_t a; }
    #elif V == 3
        #define X_FIELDS { uint32_t a; uint16_t b; uint8_t arr[6]; \
            vec_t at; switch_t mode; }
    #elif V == 4
        #define X_FIELDS { uint32_t a; uint8_t arr[4]; vec_t at; \
            switch_t mode; }
    #elif V == 5
        #define X_FIELDS { uint32_t a; uint16_t b; uint8_t arr[2]; \
            vec_t at; switch_t mode; }
    #elif V == 6
        #define X_FIELDS { uint32_t a; uint16_t bee; uint8_t arr[4]; \
            vec_t at; switch_t mode; }
    #elif V == 7
        #define X_FIELDS { uint32_t a; uint32_t b; uint8_t arr[4]; \
            vec_t at; switch_t mode; }
    #elif V == 13
        #define X_FIELDS { uint32_t a; uint16_t b; uint8_t arr[4]; \
            switch_t mode; vec_t at; }
    #else
        #define X_FIELDS { uint32_t a; uint16_t b; uint8_t arr[4]; \
            vec_t at; switch_t mode; }
    #endif

BRIDGE_SCHEMA_STRUCT_TYPE(x_t, X_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(x_t, X_FIELDS);
    #if V == 21
        #define X3_FIELDS { uint32_t a; uint64_t b; }
BRIDGE_SCHEMA_STRUCT_TYPE(x3_t, X3_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(x3_t, X3_FIELDS);
    #endif

typedef struct sums {
    uint32_t a;
    uint32_t b;
    uint32_t arr;
    uint32_t at_x;
    uint32_t at_y;
    uint32_t mode;
    uint32_t more;
} sums_t;

extern uint32_t x_slot;
extern uint32_t reading;

uint32_t collect_values(const test_api_t *api, const x_t **out,
    uint32_t room);

#endif
