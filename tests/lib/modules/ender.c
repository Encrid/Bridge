/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_offsets.h"
#include "game.h"
#include "offsets_idle.h"

BRIDGE_EXPORT(stop) int stop(void)
{
    return (BRIDGE_END);
}

#ifdef HEARD

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};

    terms[0].slot = BRIDGE_COMPONENT("game:Marked", marked_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    BRIDGE_OBSERVER("stop", BRIDGE_ON_ENTER, terms, 1, nullptr, 0, nullptr, 0);
    return (0);
}

#else

BRIDGE_EXPORT(init) int init(void)
{
    int none = 0;

    BRIDGE_ADD_SYSTEM("stop", "update", &none, 0, nullptr, 0, nullptr, 0);
    return (INIT_STATUS);
}

#endif
