/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/toggle/toggle.h -- what the sources of ntoggle
** share: the three components, how many entities the build has, and what
** init was answered.
*/

#ifndef TEST_NTOGGLE_H
    #define TEST_NTOGGLE_H

    #include "test_module.h"
    #include "say.h"

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

/*
** What a look at T counted: the rows that are on, and the sum of their
** values.
*/
typedef struct {
    uint32_t on;
    uint32_t total;
} seen_t;

/*
** The slots of A, B and T; the query of the systems that read T and of
** those that write it; the entities, with room for those `respawn_half`
** makes; and how many of those there are.
*/
extern uint32_t ntoggle_a;
extern uint32_t ntoggle_b;
extern uint32_t ntoggle_t;
extern uint32_t ntoggle_reading;
extern uint32_t ntoggle_writing;
extern uint64_t ntoggle_ids[2 * COUNT];
extern uint32_t ntoggle_more;

#endif
