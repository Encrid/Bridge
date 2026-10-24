/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_offsets.h"
#include "game.h"

static uint32_t single;

BRIDGE_EXPORT(init) int init(void)
{
    single = BRIDGE_COMPONENT("game:S", marked_t, BRIDGE_STRIP,
        BRIDGE_SINGLETON);
    return (0);
}

BRIDGE_EXPORT(reload) int reload(void)
{
    init();
#ifdef SECOND
    marked_t value = { 2 };

    BRIDGE_ATTACH(0, single, value);
#endif
    return (0);
}

BRIDGE_EXPORT(shutdown) int shutdown(void)
{
    marked_t value = { 1 };

    BRIDGE_ATTACH(0, single, value);
    return (0);
}
