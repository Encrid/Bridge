/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_LANE_H
    #define TEST_LANE_H

    #include "bridge_schema.h"

    #define LANE_VALUE_FIELDS { uint64_t v; }
    #define LANE_SEEN_FIELDS { uint32_t frame; }

BRIDGE_SCHEMA_STRUCT_TYPE(lane_value_t, LANE_VALUE_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(lane_value_t, LANE_VALUE_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(lane_seen_t, LANE_SEEN_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(lane_seen_t, LANE_SEEN_FIELDS);

#endif
