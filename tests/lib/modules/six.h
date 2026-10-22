/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_SIX_H
    #define TEST_SIX_H

    #include <stdint.h>

    #include "bridge_schema.h"

    #define SIX_FIELDS { uint32_t x; }

BRIDGE_SCHEMA_STRUCT_TYPE(six_t, SIX_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(six_t, SIX_FIELDS);

#endif
