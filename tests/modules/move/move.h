/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/move/move.h -- what the sources of nmove share:
** its two components, and what init was answered.
*/

#ifndef TEST_NMOVE_H
    #define TEST_NMOVE_H

    #include "test_module.h"

    #define A_FIELDS { uint32_t v; }
    #define X_FIELDS { uint32_t v; }

BRIDGE_SCHEMA_STRUCT_TYPE(a_t, A_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(a_t, A_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(x_t, X_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(x_t, X_FIELDS);

constexpr uint32_t COUNT = 6;

/*
** The slots of st:A and st:X, the query that reads X, and the entities.
*/
extern uint32_t a_slot;
extern uint32_t x_slot;
extern uint32_t reading;
extern uint64_t ids[COUNT];

#endif
