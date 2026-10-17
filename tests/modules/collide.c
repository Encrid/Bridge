/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_offsets.h"

#if V == 1
    #define Y_FIELDS { uint32_t v; uint8_t NAME; }
#elif V == 2 || V == 5
    #define Y_FIELDS { float v; uint8_t NAME; }
#endif
#if V <= 2 || V == 5
BRIDGE_SCHEMA_STRUCT_TYPE(y_t, Y_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(y_t, Y_FIELDS);
    #define Y_KIND BRIDGE_STRUCT
#elif V == 3
BRIDGE_SCHEMA_ENUM_TYPE(y_t, uint8_t, { NAME });
BRIDGE_SCHEMA_ENUM_DESC(y_t, uint8_t, { NAME });
    #define Y_KIND BRIDGE_ENUM
#else
BRIDGE_SCHEMA_ENUM_TYPE(y_t, uint16_t, { NAME });
BRIDGE_SCHEMA_ENUM_DESC(y_t, uint16_t, { NAME });
    #define Y_KIND BRIDGE_ENUM
#endif
#if V % 2
    #define X_FIELDS { y_t y; }
    #define Y_KEPT BRIDGE_TABLE
#else
    #define X_FIELDS { y_t y; uint32_t added; }
    #define Y_KEPT BRIDGE_SINGLETON
#endif

BRIDGE_SCHEMA_STRUCT_TYPE(x_t, X_FIELDS);
BRIDGE_SCHEMA_STRUCT_DESC(x_t, X_FIELDS);

static uint32_t x;

BRIDGE_EXPORT(reload) int reload(void)
{
    BRIDGE_DECLARE(Y_KIND, "st:y_t", y_t, BRIDGE_STRIP, Y_KEPT);
    x = BRIDGE_COMPONENT("st:X", x_t, BRIDGE_STRIP, BRIDGE_TABLE);
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    x_t value = {};

    reload();
    BRIDGE_ATTACH(bridge_spawn(), x, value);
    return (0);
}

BRIDGE_EXPORT(shutdown) int shutdown(void)
{
    return (0);
}
