/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_AIM_H
    #define TEST_AIM_H

    #include "bridge_schema.h"

    #define AIM_FIELDS { uint32_t at; }

BRIDGE_SCHEMA_STRUCT_TYPE(aim_t, AIM_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(aim_t, AIM_FIELDS);

#endif
