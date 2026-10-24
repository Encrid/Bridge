/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_PACKED_H
    #define TEST_PACKED_H
    #include "bridge_schema.h"
    #define COUNT_FIELDS { uint32_t n; }

BRIDGE_SCHEMA_STRUCT_TYPE(count_t, COUNT_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(count_t, COUNT_FIELDS);

#endif
