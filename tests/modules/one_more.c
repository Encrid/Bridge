/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_offsets.h"
#include "game.h"
#include "offsets_idle.h"

static uint32_t position;

BRIDGE_EXPORT(add) int add(void)
{
    bridge_attach(bridge_spawn(), position, BRIDGE_ZEROED);
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    int none = 0;

    position = BRIDGE_COMPONENT("st:Position", position_t, BRIDGE_DELETE,
        BRIDGE_TABLE);
    BRIDGE_ADD_SYSTEM("add", "spawn", &none, 0, &position, 1, nullptr, 0);
    return (0);
}
