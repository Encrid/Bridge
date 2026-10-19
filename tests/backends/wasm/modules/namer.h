/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_NAMER_H
    #define TEST_NAMER_H

    #include "bridge_abi.h"
    #include "bridge_schema.h"

    #define NAME_FIELDS { bridge_text_t text; }

BRIDGE_SCHEMA_STRUCT_TYPE(name_t, NAME_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(name_t, NAME_FIELDS);

#endif
