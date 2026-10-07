/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/native/nticker.h -- the one component of nticker.c: a
** count.
*/

#ifndef TEST_NTICKER_H
    #define TEST_NTICKER_H

    #include "bridge_schema.h"

    #define TICK_FIELDS { uint32_t n; }

BRIDGE_SCHEMA_STRUCT_TYPE(tick_t, TICK_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(tick_t, TICK_FIELDS);

#endif
