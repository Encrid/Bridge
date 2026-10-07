/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/wasm/settings/settings.h -- what the sources of the
** settings module share: the component its settings are, with a field of
** every kind a setting can be, and what init was answered for it.
**
** config.md, *A module's own settings are a component*
*/

#ifndef TEST_SETTINGS_H
    #define TEST_SETTINGS_H

    #include <stdint.h>
    #include "bridge_abi.h"
    #include "bridge_schema.h"

    #define MODE_VALUES { OFF, ON, ADAPTIVE }
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
    }

BRIDGE_SCHEMA_ENUM_TYPE(mode_t, int8_t, MODE_VALUES);
BRIDGE_SCHEMA_ENUM_DESC(mode_t, int8_t, MODE_VALUES);
BRIDGE_SCHEMA_BITMASK_TYPE(flags_t, uint32_t, FLAGS_VALUES);
BRIDGE_SCHEMA_BITMASK_DESC(flags_t, uint32_t, FLAGS_VALUES);
BRIDGE_SCHEMA_STRUCT_TYPE(settings_t, SETTINGS_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(settings_t, SETTINGS_FIELDS);

extern uint32_t settings;

#endif
