/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_NDIRECT_H
    #define TEST_NDIRECT_H

    #include "bridge_native.h"
    #include "native_say.h"

    #define A_FIELDS { uint32_t v; }
    #define D_FIELDS { uint32_t v; }

BRIDGE_SCHEMA_STRUCT_TYPE(a_t, A_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(a_t, A_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(d_t, D_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(d_t, D_FIELDS);

    #ifdef BIG
constexpr uint32_t COUNT = 204'800;
constexpr uint32_t STEP = 1;
    #else
constexpr uint32_t COUNT = 6;
constexpr uint32_t STEP = 2;
    #endif

typedef struct {
    uint32_t alive;
    uint32_t found;
    uint32_t total;
} reached_t;

extern uint32_t ndirect_a;
extern uint32_t ndirect_d;
extern uint32_t ndirect_in_table;
extern uint64_t ndirect_ids[COUNT + 2];
extern uint64_t ndirect_mixed[COUNT];

    #ifndef BIG
extern uint32_t ndirect_on_d;
extern uint32_t ndirect_on_both;
    #endif

#endif
