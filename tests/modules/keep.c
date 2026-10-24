/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_offsets.h"
#include "game.h"

BRIDGE_EXPORT(heard) int heard(void)
{
    return (0);
}

BRIDGE_EXPORT(tick) int tick(void)
{
    BRIDGE_LOG(BRIDGE_LOG_INFO, "tick", 0, 0);
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    int none = 0;

    terms[0].slot = BRIDGE_COMPONENT("game:Position", position_t,
        BRIDGE_STRIP, BRIDGE_TABLE);
    BRIDGE_OBSERVER("heard", BRIDGE_ON_ENTER, terms, 1, nullptr, 0, nullptr, 0);
    BRIDGE_ADD_SYSTEM("tick", "update", &none, 0, nullptr, 0, nullptr, 0);
    return (0);
}

BRIDGE_EXPORT(reload) int reload(void)
{
#ifdef FAIL_RELOAD
    return (7);
#else
    return (init());
#endif
}

BRIDGE_EXPORT(shutdown) int shutdown(void)
{
    return (0);
}
