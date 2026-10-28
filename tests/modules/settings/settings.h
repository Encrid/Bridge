/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#ifndef TEST_SETTINGS_H
    #define TEST_SETTINGS_H

    #include <stdint.h>
    #include "bridge_abi.h"
    #include "bridge_schema.h"

    #define MODE_VALUES { OFF, ON, ADAPTIVE }
    #ifdef MORE
        #define E16_VALUES { E16_LOW = -1, E16_HIGH = 1 }
        #define E32_VALUES { E32_LOW = -1, E32_HIGH = 1 }
        #define EU8_VALUES { EU8_LOW, EU8_HIGH }
        #define PAIR_FIELDS { int32_t a; int32_t b; }
        #define MASK64_VALUES { MASK64_LOW = 1, \
            MASK64_TOP = 0x8000000000000000 }
        #define SETTINGS_WIDE e16_t e16; e32_t e32; eu8_t eu8; \
            bridge_text_t names[2]; pair_t pair; mask64_t mask64;
    #else
        #define SETTINGS_WIDE
    #endif
    #define FLAGS_VALUES { LOUD = 1, WIDE = 2 }
    #define SETTINGS_FIELDS { \
        bool vsync; \
        int32_t width; \
        float scale; \
        mode_t mode; \
        char title[16]; \
        bridge_text_t path; \
        uint8_t small; \
        flags_t flags; \
        double ratio; \
        int64_t big; \
        SETTINGS_WIDE \
    }

BRIDGE_SCHEMA_ENUM_TYPE(mode_t, int8_t, MODE_VALUES);
BRIDGE_SCHEMA_ENUM_DESC(mode_t, int8_t, MODE_VALUES);
BRIDGE_SCHEMA_BITMASK_TYPE(flags_t, uint32_t, FLAGS_VALUES);
BRIDGE_SCHEMA_BITMASK_DESC(flags_t, uint32_t, FLAGS_VALUES);
    #ifdef MORE
BRIDGE_SCHEMA_ENUM_TYPE(e16_t, int16_t, E16_VALUES);
BRIDGE_SCHEMA_ENUM_DESC(e16_t, int16_t, E16_VALUES);
BRIDGE_SCHEMA_ENUM_TYPE(e32_t, int32_t, E32_VALUES);
BRIDGE_SCHEMA_ENUM_DESC(e32_t, int32_t, E32_VALUES);
BRIDGE_SCHEMA_ENUM_TYPE(eu8_t, uint8_t, EU8_VALUES);
BRIDGE_SCHEMA_ENUM_DESC(eu8_t, uint8_t, EU8_VALUES);
BRIDGE_SCHEMA_STRUCT_TYPE(pair_t, PAIR_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(pair_t, PAIR_FIELDS);
BRIDGE_SCHEMA_BITMASK_TYPE(mask64_t, uint64_t, MASK64_VALUES);
BRIDGE_SCHEMA_BITMASK_DESC(mask64_t, uint64_t, MASK64_VALUES);
    #endif
BRIDGE_SCHEMA_STRUCT_TYPE(settings_t, SETTINGS_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(settings_t, SETTINGS_FIELDS);

extern uint32_t settings;

#endif
