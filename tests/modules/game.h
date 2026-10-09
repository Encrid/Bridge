/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/game.h -- the schema several test modules share: what a
** game would publish beside its guarded.
**
** schema.md, *There is no schema format*
*/

#ifndef TEST_GAME_H
    #define TEST_GAME_H

    #include "bridge_schema.h"

    #define POSITION_FIELDS { float x; float y; }
    #define MARKED_FIELDS { uint32_t by; }

BRIDGE_SCHEMA_STRUCT_TYPE(position_t, POSITION_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(position_t, POSITION_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(marked_t, MARKED_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(marked_t, MARKED_FIELDS);

#endif
