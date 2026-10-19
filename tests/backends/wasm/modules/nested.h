/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_NESTED_H
    #define TEST_NESTED_H

    #include "vec2.h"

    #define TEAM_VALUES { RED, GREEN, BLUE }
    #define FLAGS_VALUES { SOLID = 1, HIDDEN = 2 }
    #define BODY_FIELDS { vec2_t at; team_t team; flags_t flags; float mass; }

BRIDGE_SCHEMA_ENUM_TYPE(team_t, uint8_t, TEAM_VALUES);
BRIDGE_SCHEMA_ENUM_DESC(team_t, uint8_t, TEAM_VALUES);
BRIDGE_SCHEMA_BITMASK_TYPE(flags_t, uint16_t, FLAGS_VALUES);
BRIDGE_SCHEMA_BITMASK_DESC(flags_t, uint16_t, FLAGS_VALUES);
BRIDGE_SCHEMA_STRUCT_TYPE(body_t, BODY_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(body_t, BODY_FIELDS);

#endif
