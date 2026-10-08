/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/wasm/user.h -- the component of user.c, with a field of
** a type that another module declares.
*/

#ifndef TEST_USER_H
    #define TEST_USER_H

    #include "vec2.h"

    #define BODY_FIELDS { vec2_t at; float mass; }

BRIDGE_SCHEMA_STRUCT_TYPE(body_t, BODY_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(body_t, BODY_FIELDS);

#endif
