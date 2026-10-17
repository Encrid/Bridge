/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "test_offsets.h"
#include "game.h"

BRIDGE_EXPORT(push) int push(void)
{
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_WRITE, BRIDGE_AND, 0 }};

    terms[0].slot = BRIDGE_COMPONENT("game:Position", position_t,
        BRIDGE_STRIP, BRIDGE_TABLE);
    BRIDGE_ADD_SYSTEM("push", "update", terms, 1, nullptr, 0);
    return (0);
}

BRIDGE_EXPORT(reload) int reload(void)
{
    return (init());
}

BRIDGE_EXPORT(shutdown) int shutdown(void)
{
    return (0);
}
