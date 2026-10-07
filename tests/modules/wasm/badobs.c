/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/wasm/badobs.c -- names an observer init, which is an
** export that already means something.
*/

#include "bridge_wasm.h"
#include "game.h"
#include "wasm_idle.h"

/*
** init
**
** Declares the position, and an observer of it under the name init.
*/
BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};

    terms[0].slot = BRIDGE_COMPONENT("game:Position", position_t,
        BRIDGE_STRIP, BRIDGE_TABLE);
    BRIDGE_OBSERVER("init", BRIDGE_ON_ENTER, terms, 1, nullptr, 0);
    return (0);
}
