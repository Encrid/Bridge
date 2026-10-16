/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_offsets.h"
#include "game.h"
#include "offsets_idle.h"

BRIDGE_EXPORT(first) int first(void)
{
    return (7);
}

BRIDGE_EXPORT(second) int second(void)
{
    return (7);
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};

    terms[0].slot = BRIDGE_COMPONENT("game:Marked", marked_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    BRIDGE_OBSERVER("first", BRIDGE_ON_ENTER, terms, 1, nullptr, 0);
    BRIDGE_OBSERVER("second", BRIDGE_ON_ENTER, terms, 1, nullptr, 0);
    return (0);
}
