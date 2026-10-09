/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/stepper.h -- the one component of stepper.c: a
** count.
*/

#ifndef TEST_STEPPER_H
    #define TEST_STEPPER_H

    #include "bridge_schema.h"

    #define TICK_FIELDS { uint32_t n; }

BRIDGE_SCHEMA_STRUCT_TYPE(tick_t, TICK_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(tick_t, TICK_FIELDS);

#endif
