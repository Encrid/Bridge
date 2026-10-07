/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/wasm/hit/hit.h -- what the sources of the hit module
** share: its component, and what init was answered.
*/

#ifndef TEST_HIT_H
    #define TEST_HIT_H

    #include <stdint.h>
    #include "bridge_schema.h"

    #define HIT_FIELDS { uint32_t by; }

BRIDGE_SCHEMA_STRUCT_TYPE(hit_t, HIT_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(hit_t, HIT_FIELDS);

extern uint32_t position;
extern uint32_t hit;
extern uint32_t hits;
extern uint32_t bodies;

#endif
