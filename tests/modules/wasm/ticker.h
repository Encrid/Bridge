/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/wasm/ticker.h -- the components of ticker.c. WIDE changes
** the body of the one it counts in.
*/

#ifndef TEST_TICKER_H
    #define TEST_TICKER_H

    #include "bridge_schema.h"

    #ifdef WIDE
        #define TICK_FIELDS { uint32_t n; uint32_t more; }
    #else
        #define TICK_FIELDS { uint32_t n; }
    #endif
    #define EXTRA_FIELDS { uint32_t unused; }

BRIDGE_SCHEMA_STRUCT_TYPE(tick_t, TICK_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(tick_t, TICK_FIELDS);
BRIDGE_SCHEMA_STRUCT_TYPE(extra_t, EXTRA_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(extra_t, EXTRA_FIELDS);

#endif
