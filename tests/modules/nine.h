/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_NINE_H
    #define TEST_NINE_H

    #include <stdint.h>

    #include "bridge_schema.h"

    #ifdef WIDE
        #define NINE_FIELDS { int32_t width; int32_t extra; }
    #else
        #define NINE_FIELDS { int32_t width; }
    #endif

BRIDGE_SCHEMA_STRUCT_TYPE(nine_t, NINE_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(nine_t, NINE_FIELDS);

#endif
