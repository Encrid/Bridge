/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_CHAIN_H
    #define TEST_CHAIN_H

    #include "bridge_schema.h"

    #define LINK_FIELDS { uint32_t n; }

BRIDGE_SCHEMA_STRUCT_TYPE(link_t, LINK_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(link_t, LINK_FIELDS);

#endif
