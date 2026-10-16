/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_offsets.h"
#include "offsets_idle.h"

BRIDGE_EXPORT(first) int first(void)
{
    BRIDGE_LOG(BRIDGE_LOG_INFO, "at", 0, 0);
    return (0);
}

BRIDGE_EXPORT(second) int second(void)
{
    BRIDGE_LOG(BRIDGE_LOG_INFO, "at", 0, 0);
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    int none = 0;

    BRIDGE_ADD_SYSTEM("first", "update", &none, 0, nullptr, 0);
    BRIDGE_ADD_SYSTEM("second", "update", &none, 0, nullptr, 0);
    return (0);
}
