/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_SPOT_H
    #define TEST_SPOT_H

    #include <stdint.h>

    #include "bridge_schema.h"

    #define SPOT_FIELDS { uint32_t x; uint32_t y; }

BRIDGE_SCHEMA_STRUCT_TYPE(spot_t, SPOT_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(spot_t, SPOT_FIELDS);

#endif
