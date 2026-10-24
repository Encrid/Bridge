/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_offsets.h"
#include "game.h"

static uint32_t marked;

BRIDGE_EXPORT(init) int init(void)
{
    marked = BRIDGE_COMPONENT("game:Marked", marked_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    return (0);
}

BRIDGE_EXPORT(reload) int reload(void)
{
    marked_t value = { 1 };

    init();
    BRIDGE_ATTACH(bridge_spawn(), marked, value);
    return (0);
}

BRIDGE_EXPORT(shutdown) int shutdown(void)
{
    return (0);
}
