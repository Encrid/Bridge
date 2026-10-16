/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_offsets.h"
#include "game.h"
#include "offsets_idle.h"

static uint32_t one;
static uint32_t two;

BRIDGE_EXPORT(first) int first(void)
{
    bridge_attach(bridge_spawn(), one, BRIDGE_ZEROED);
    return (0);
}

BRIDGE_EXPORT(second) int second(void)
{
    bridge_attach(bridge_spawn(), two, BRIDGE_ZEROED);
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    int none = 0;

    one = BRIDGE_DECLARE_TAG("game:One", BRIDGE_STRIP, BRIDGE_TABLE);
    two = BRIDGE_DECLARE_TAG("game:Two", BRIDGE_STRIP, BRIDGE_TABLE);
    BRIDGE_ADD_SYSTEM("first", "update", &none, 0, &one, 1);
    BRIDGE_ADD_SYSTEM("second", "update", &none, 0, &two, 1);
    return (0);
}
