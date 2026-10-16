/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_offsets.h"
#include "settings.h"

uint32_t settings;

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_OPTIONAL, 0 }};

    BRIDGE_DECLARE(BRIDGE_ENUM, "game:mode_t", mode_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    BRIDGE_DECLARE(BRIDGE_BITMASK, "game:flags_t", flags_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
#ifdef MORE
    BRIDGE_DECLARE(BRIDGE_ENUM, "game:e16_t", e16_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    BRIDGE_DECLARE(BRIDGE_ENUM, "game:e32_t", e32_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    BRIDGE_DECLARE(BRIDGE_ENUM, "game:eu8_t", eu8_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    BRIDGE_DECLARE(BRIDGE_STRUCT, "game:pair_t", pair_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
#endif
    settings = BRIDGE_COMPONENT("game:Settings", settings_t, BRIDGE_DELETE,
        KEPT);
    terms[0].slot = settings;
    BRIDGE_ADD_SYSTEM("show", "update", terms, 1, nullptr, 0);
    return (0);
}

BRIDGE_EXPORT(reload) int reload(void)
{
#ifdef DROP
    return (0);
#else
    return (init());
#endif
}

BRIDGE_EXPORT(shutdown) int shutdown(void)
{
    return (0);
}
