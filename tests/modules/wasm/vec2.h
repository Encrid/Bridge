/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/wasm/vec2.h -- a struct that is a field's type in the
** components of other modules: typer.c declares it, user.c and nested.c
** have a field of it.
*/

#ifndef TEST_VEC2_H
    #define TEST_VEC2_H

    #include "bridge_schema.h"

    #define VEC2_FIELDS { float x; float y; }

BRIDGE_SCHEMA_STRUCT_TYPE(vec2_t, VEC2_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(vec2_t, VEC2_FIELDS);

#endif
