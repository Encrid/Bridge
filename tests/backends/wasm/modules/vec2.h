/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_VEC2_H
    #define TEST_VEC2_H

    #include "bridge_schema.h"

    #define VEC2_FIELDS { float x; float y; }

BRIDGE_SCHEMA_STRUCT_TYPE(vec2_t, VEC2_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(vec2_t, VEC2_FIELDS);

#endif
