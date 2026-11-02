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

#ifdef WIDE_TEAM_TYPE
BRIDGE_SCHEMA_ENUM_TYPE(team_t, uint16_t, TEAM_VALUES);
#else
BRIDGE_SCHEMA_ENUM_TYPE(team_t, uint8_t, TEAM_VALUES);
#endif
#ifdef WIDE_TEAM_DESC
BRIDGE_SCHEMA_ENUM_DESC(team_t, uint16_t, TEAM_VALUES);
#else
BRIDGE_SCHEMA_ENUM_DESC(team_t, uint8_t, TEAM_VALUES);
#endif
#ifdef WIDE_FLAGS_TYPE
BRIDGE_SCHEMA_BITMASK_TYPE(flags_t, uint32_t, FLAGS_VALUES);
#else
BRIDGE_SCHEMA_BITMASK_TYPE(flags_t, uint16_t, FLAGS_VALUES);
#endif
#ifdef WIDE_FLAGS_DESC
BRIDGE_SCHEMA_BITMASK_DESC(flags_t, uint32_t, FLAGS_VALUES);
#else
BRIDGE_SCHEMA_BITMASK_DESC(flags_t, uint16_t, FLAGS_VALUES);
#endif
BRIDGE_SCHEMA_STRUCT_TYPE(body_t, BODY_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(body_t, BODY_FIELDS);

#endif
