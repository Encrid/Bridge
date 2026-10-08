/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/wasm/retype.h -- the layout retype.c gives a component
** name, which nobody else gives it.
*/

#ifndef TEST_RETYPE_H
    #define TEST_RETYPE_H

    #include "bridge_schema.h"

    #define RETYPED_FIELDS { uint32_t a; uint32_t b; uint32_t c; }

BRIDGE_SCHEMA_STRUCT_TYPE(retyped_t, RETYPED_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(retyped_t, RETYPED_FIELDS);

#endif
