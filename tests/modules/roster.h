/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_ROSTER_H
    #define TEST_ROSTER_H

    #include "bridge_schema.h"

    #define POSITION_FIELDS { float x; float y; }
    #define PRIVATE_FIELDS { uint32_t secret; }
    #define ROSTER_FIELDS { uint64_t ids[5]; }

BRIDGE_SCHEMA_STRUCT_TYPE(position_t, POSITION_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(position_t, POSITION_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(private_t, PRIVATE_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(private_t, PRIVATE_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(roster_t, ROSTER_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(roster_t, ROSTER_FIELDS);

#endif
