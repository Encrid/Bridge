/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_NTOGGLE_H
    #define TEST_NTOGGLE_H

    #include "bridge_native.h"
    #include "native_say.h"

    #define A_FIELDS { uint32_t v; }
    #define B_FIELDS { uint32_t v; }
    #define T_FIELDS { uint32_t v; }

BRIDGE_SCHEMA_STRUCT_TYPE(a_t, A_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(a_t, A_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(b_t, B_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(b_t, B_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(t_t, T_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(t_t, T_FIELDS);

    #ifdef BIG
constexpr uint32_t COUNT = 204'800;
    #else
constexpr uint32_t COUNT = 6;
    #endif

typedef struct {
    uint32_t on;
    uint32_t total;
} seen_t;

extern uint32_t ntoggle_a;
extern uint32_t ntoggle_b;
extern uint32_t ntoggle_t;
extern uint32_t ntoggle_reading;
extern uint32_t ntoggle_writing;
extern uint64_t ntoggle_ids[2 * COUNT];
extern uint32_t ntoggle_more;

#endif
