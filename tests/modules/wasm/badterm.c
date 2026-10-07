/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge.
**
** All rights reserved.
**
** tests/modules/wasm/badterm.c -- hands over a term whose operator is none
** of the three there are: its load is refused.
*/

#include "bridge_wasm.h"
#include "game.h"
#include "wasm_idle.h"

/*
** any
**
** Does nothing.
*/
BRIDGE_EXPORT(any) int any(void)
{
    return (0);
}

/*
** init
**
** Declares the position, and a system whose one term has the operator 4.
*/
BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, (enum bridge_oper)4, 0 }};

    terms[0].slot = BRIDGE_COMPONENT("game:Position", position_t,
        BRIDGE_STRIP, BRIDGE_TABLE);
    BRIDGE_ADD_SYSTEM("any", "update", terms, 1, nullptr, 0);
    return (0);
}
