/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/packaged/npack.h -- the component of npack.c.
*/

#ifndef TEST_NPACK_H
    #define TEST_NPACK_H
    #include "bridge_schema.h"
    #define COUNT_FIELDS { uint32_t n; }

BRIDGE_SCHEMA_STRUCT_TYPE(count_t, COUNT_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(count_t, COUNT_FIELDS);

#endif
