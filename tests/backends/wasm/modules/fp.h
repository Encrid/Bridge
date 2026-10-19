/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_FP_H
    #define TEST_FP_H

    #include "bridge_schema.h"

    #define POSITION_FIELDS { float x; float y; float z; }

BRIDGE_SCHEMA_STRUCT_TYPE(position_t, POSITION_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(position_t, POSITION_FIELDS);

#endif
