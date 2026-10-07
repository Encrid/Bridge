/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/wasm/namer.h -- the component of namer.c: a string, by
** where its bytes are in the persistent arena and how many they are.
*/

#ifndef TEST_NAMER_H
    #define TEST_NAMER_H

    #include "bridge_abi.h"
    #include "bridge_schema.h"

    #define NAME_FIELDS { bridge_text_t text; }

BRIDGE_SCHEMA_STRUCT_TYPE(name_t, NAME_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(name_t, NAME_FIELDS);

#endif
