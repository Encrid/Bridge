/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/roster.h -- what the keeper and the counter share: a
** position, something private, and a list of ids to ask after.
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
