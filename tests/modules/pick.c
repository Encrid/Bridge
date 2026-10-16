/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_offsets.h"
#include "game.h"
#include "offsets_idle.h"

BRIDGE_EXPORT(look) int look(void)
{
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[2] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 },
        { 0, BRIDGE_READ, BRIDGE_OPTIONAL, 0 }};

    terms[0].slot = BRIDGE_COMPONENT("game:Picked", marked_t, BRIDGE_STRIP,
        BRIDGE_SPARSE_SET);
    terms[1].slot = BRIDGE_COMPONENT("game:Position", position_t,
        BRIDGE_STRIP, BRIDGE_TABLE);
    BRIDGE_ADD_SYSTEM("look", "update", terms, 2, nullptr, 0);
    return (0);
}
