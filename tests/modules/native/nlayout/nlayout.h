/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/native/nlayout/nlayout.h -- what the sources of nlayout
** share: the component X, whose struct this build states, the two types
** its fields name, and what init was answered. V says which struct:
**
**     V  0   a, b, arr[4], at { x, y }, mode { OFF, ON, AUTO }
**        1   a field added
**        2   the fields in another order
**        3   arr made longer
**        4   b removed
**        5   arr made shorter
**        6   b renamed
**        7   b of another type
**        8   the struct `at` names changed: its fields in another order,
**            and one added
**        9   the enum's constants in another order, so that two have
**            another number
**       10   a constant added to the enum
**       11   a field removed from the struct `at` names
**       12   a constant removed from the enum
**       13   the struct `at` named declared as a tag, and X without that
**            field
**
** lifetime.md, *When a component layout changed*
*/

#ifndef TEST_NLAYOUT_H
    #define TEST_NLAYOUT_H

    #include "bridge_native.h"

    #if V == 9
        #define SWITCH_VALUES { OFF, AUTO, ON }
    #elif V == 10
        #define SWITCH_VALUES { OFF, ON, AUTO, MAX }
    #elif V == 12
        #define SWITCH_VALUES { OFF, ON }
    #else
        #define SWITCH_VALUES { OFF, ON, AUTO }
    #endif

BRIDGE_SCHEMA_ENUM_TYPE(switch_t, int8_t, SWITCH_VALUES);
BRIDGE_SCHEMA_ENUM_DESC(switch_t, int8_t, SWITCH_VALUES);

    #if V == 8
        #define VEC_FIELDS { uint16_t y; uint16_t x; uint16_t z; }
    #elif V == 11
        #define VEC_FIELDS { uint16_t x; }
    #else
        #define VEC_FIELDS { uint16_t x; uint16_t y; }
    #endif

/*
** The build that declares st:vec_t as a tag hands no descriptor of it.
*/
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

/*
** What `look` says: every field this build has, summed over the values
** there are. `more` is the sum of the field a build added.
*/
typedef struct sums {
    uint32_t a;
    uint32_t b;
    uint32_t arr;
    uint32_t at_x;
    uint32_t at_y;
    uint32_t mode;
    uint32_t more;
} sums_t;

/*
** The slot of st:X, and the query that reads it.
*/
extern uint32_t x_slot;
extern uint32_t reading;

/*
** collect_values
**
** Puts in `out` where the values of X are, however this build keeps it, and
** answers how many: `room` at the most.
*/
uint32_t collect_values(const bridge_api_t *api, const x_t **out,
    uint32_t room);

#endif
