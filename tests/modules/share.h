/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_SHARE_H
    #define TEST_SHARE_H

    #include "bridge_schema.h"

    #if WIDE
        #define SHARED_FIELDS { int32_t x; int32_t y; }
    #else
        #define SHARED_FIELDS { int32_t x; }
    #endif

BRIDGE_SCHEMA_STRUCT_TYPE(shared_t, SHARED_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(shared_t, SHARED_FIELDS);

#endif
