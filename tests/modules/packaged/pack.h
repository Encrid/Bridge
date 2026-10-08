/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/packaged/pack.h -- the component of pack.c.
*/

#ifndef TEST_PACK_H
    #define TEST_PACK_H
    #include "bridge_schema.h"
    #define SUM_FIELDS { uint32_t n; }

BRIDGE_SCHEMA_STRUCT_TYPE(sum_t, SUM_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(sum_t, SUM_FIELDS);

#endif
