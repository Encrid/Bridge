/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
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

extern uint32_t a_slot;
extern uint32_t x_slot;
extern uint32_t reading;
extern uint64_t ids[COUNT];

#endif
