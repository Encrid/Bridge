/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_STEPPER_H
    #define TEST_STEPPER_H

    #include "bridge_schema.h"

    #define TICK_FIELDS { uint32_t n; }

BRIDGE_SCHEMA_STRUCT_TYPE(tick_t, TICK_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(tick_t, TICK_FIELDS);

#endif
