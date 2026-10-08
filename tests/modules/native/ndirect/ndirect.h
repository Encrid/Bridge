/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/ndirect/ndirect.h -- what the sources of ndirect
** share: the two components, how many entities the build has, and what
** init was answered.
*/

#ifndef TEST_NDIRECT_H
    #define TEST_NDIRECT_H

    #include "bridge_native.h"
    #include "native/native_say.h"

    #define A_FIELDS { uint32_t v; }
    #define D_FIELDS { uint32_t v; }

BRIDGE_SCHEMA_STRUCT_TYPE(a_t, A_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(a_t, A_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(d_t, D_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(d_t, D_FIELDS);

/*
** How many entities there are, and one in how many of them is given D.
*/
    #ifdef BIG
constexpr uint32_t COUNT = 204'800;
constexpr uint32_t STEP = 1;
    #else
constexpr uint32_t COUNT = 6;
constexpr uint32_t STEP = 2;
    #endif

/*
** What reaching entities by ids that are held counted: those alive, those
** of them given D, and the sum of their D.
*/
typedef struct {
    uint32_t alive;
    uint32_t found;
    uint32_t total;
} reached_t;

/*
** The slots of A and of D, the query of `look_b`, the entities, with room
** for the two that `respawn_half` makes, and the ids of entities in no
** order.
*/
extern uint32_t ndirect_a;
extern uint32_t ndirect_d;
extern uint32_t ndirect_in_table;
extern uint64_t ndirect_ids[COUNT + 2];
extern uint64_t ndirect_mixed[COUNT];

/*
** The observer of D, and the observer of A and D. A build for a measurement
** has no observer.
*/
    #ifndef BIG
extern uint32_t ndirect_on_d;
extern uint32_t ndirect_on_both;
    #endif

#endif
