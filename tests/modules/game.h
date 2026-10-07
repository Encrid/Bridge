/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/modules/game.h -- the schema several test modules share: what a
** game would publish beside its wasm.
**
** schema.md, *There is no schema format*
*/

#ifndef TEST_GAME_H
    #define TEST_GAME_H

    #include "bridge_schema.h"

    #define POSITION_FIELDS { float x; float y; }
    #define MARKED_FIELDS { uint32_t by; }

BRIDGE_SCHEMA_STRUCT_TYPE(position_t, POSITION_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(position_t, POSITION_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(marked_t, MARKED_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(marked_t, MARKED_FIELDS);

#endif
