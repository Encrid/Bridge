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
    #else
        #define VEC_FIELDS { uint16_t x; uint16_t y; }
    #endif

BRIDGE_SCHEMA_STRUCT_TYPE(vec_t, VEC_FIELDS);
    #if V != 13
BRIDGE_SCHEMA_STRUCT_DESC(vec_t, VEC_FIELDS);
    #endif

    #if V == 1
        #define X_FIELDS { uint32_t a; uint16_t b; uint8_t arr[4]; \
            vec_t at; switch_t mode; uint32_t added; }
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
