/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "bridge_wasm.h"
#include "game.h"
#include "wasm_idle.h"

BRIDGE_EXPORT(any) int any(void)
{
    return (0);
}

BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, (enum bridge_oper)4, 0 }};

    terms[0].slot = BRIDGE_COMPONENT("game:Position", position_t,
        BRIDGE_STRIP, BRIDGE_TABLE);
    BRIDGE_ADD_SYSTEM("any", "update", terms, 1, nullptr, 0, nullptr, 0);
    return (0);
}
