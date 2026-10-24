/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_offsets.h"
#include "offsets_idle.h"

#ifdef BEFORE
constexpr uint32_t SECOND_BEFORE_COUNT = 1;
#else
constexpr uint32_t SECOND_BEFORE_COUNT = 0;
#endif

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
    uint32_t at_first;

    at_first = BRIDGE_ADD_SYSTEM("first", "update", &none, 0, nullptr, 0,
        nullptr, 0);
    BRIDGE_ADD_SYSTEM("second", "update", &none, 0, nullptr, 0, &at_first,
        SECOND_BEFORE_COUNT);
    return (0);
}
