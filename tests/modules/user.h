/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_USER_H
    #define TEST_USER_H

    #include "vec2.h"

    #define BODY_FIELDS { vec2_t at; float mass; }

BRIDGE_SCHEMA_STRUCT_TYPE(body_t, BODY_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(body_t, BODY_FIELDS);

#endif
