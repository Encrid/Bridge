/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/wasm/user.h -- the component of user.c, with a field of
** a type that another module declares.
*/

#ifndef TEST_USER_H
    #define TEST_USER_H

    #include "vec2.h"

    #define BODY_FIELDS { vec2_t at; float mass; }

BRIDGE_SCHEMA_STRUCT_TYPE(body_t, BODY_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(body_t, BODY_FIELDS);

#endif
