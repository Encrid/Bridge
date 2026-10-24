/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "hello.h"

#ifndef SAY_LEVEL
    #define SAY_LEVEL BRIDGE_LOG_INFO
#endif

static uint32_t calls;

BRIDGE_EXPORT(say) int say(void)
{
#ifdef REACH
    calls += (uint32_t)reach();
#endif
#ifdef ONCE
    if (calls)
        return (0);
#endif
    calls++;
#ifdef END_AT
    if (calls == END_AT)
        return (BRIDGE_END);
#endif
    BRIDGE_LOG(SAY_LEVEL, SAY, calls, 0);
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    int none = 0;

    BRIDGE_ADD_SYSTEM("say", "update", &none, 0, nullptr, 0, nullptr, 0);
    return (0);
}

BRIDGE_EXPORT(shutdown) int shutdown(void)
{
    return (0);
}

BRIDGE_EXPORT(reload) int reload(void)
{
    return (0);
}
