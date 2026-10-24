/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
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
