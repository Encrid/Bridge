/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/native/nholder.h -- the one component of nholder.c, and the
** type its field is of. That type, st:vec_t, is declared by the module
** loaded before this one: here it is only what y_t is made of.
*/

#ifndef TEST_NHOLDER_H
    #define TEST_NHOLDER_H

    #include <stdint.h>

    #include "bridge_schema.h"

    #define VEC_FIELDS { uint16_t x; uint16_t y; }
    #define Y_FIELDS { vec_t at; }

BRIDGE_SCHEMA_STRUCT_TYPE(vec_t, VEC_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(y_t, Y_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(y_t, Y_FIELDS);

#endif
