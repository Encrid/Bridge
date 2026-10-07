/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/wasm/fp.h -- game:Position with another struct body
** than game.h gives it, as a module built against an older header would.
*/

#ifndef TEST_FP_H
    #define TEST_FP_H

    #include "bridge_schema.h"

    #define POSITION_FIELDS { float x; float y; float z; }

BRIDGE_SCHEMA_STRUCT_TYPE(position_t, POSITION_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(position_t, POSITION_FIELDS);

#endif
