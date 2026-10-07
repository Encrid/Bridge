/*
** Copyright (C) 2026 Heuretes-Workshop
**
** This file is part of Bridge-ASM700.
**
** All rights reserved.
**
** tests/modules/wasm/wrong/wrong.c -- the wrong module: what it declares
** and registers before it does the one thing that is refused.
*/

#include "wasm_idle.h"
#include "wrong.h"

uint32_t wrong_position;
uint32_t wrong_thing;

/*
** wrong_do_nothing
**
** Does nothing: what a build states for the place it does no wrong in.
*/
void wrong_do_nothing(void)
{
}

/*
** act
**
** Does what this build does in a system.
*/
BRIDGE_EXPORT(act) int act(void)
{
    IN_SYSTEM();
    return (0);
}

/*
** init
**
** Declares the position and the thing, registers "act", which reads the
** position and states that it changes it, and does what this build does at
** init.
*/
BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    uint32_t changes[1];

    wrong_position = BRIDGE_COMPONENT("game:Position", position_t,
        BRIDGE_STRIP, BRIDGE_TABLE);
    wrong_thing = BRIDGE_COMPONENT("other:Thing", marked_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    terms[0].slot = wrong_position;
    changes[0] = wrong_position;
    BRIDGE_ADD_SYSTEM("act", "update", terms, 1, changes, 1);
    AT_INIT();
    return (0);
}
