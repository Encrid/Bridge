/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_SHIELD_H
    #define TEST_SHIELD_H

    #include "bridge_schema.h"

    #define SHIELD_FIELDS { uint32_t strength; }

BRIDGE_SCHEMA_STRUCT_TYPE(shield_t, SHIELD_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(shield_t, SHIELD_FIELDS);

#endif
