/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/wasm/wrong/wrong.c -- the wrong module: what it declares
** and registers before it does the one thing that is refused.
*/

#include "wasm_idle.h"
#include "wrong.h"

uint32_t wrong_position;
uint32_t wrong_thing;
uint32_t wrong_flag;

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
** Declares the position, the thing and the tag, registers "act", which
** reads the position and states that it changes it and the tag, and does
** what this build does at init.
*/
BRIDGE_EXPORT(init) int init(void)
{
    bridge_term_t terms[1] = {{ 0, BRIDGE_READ, BRIDGE_AND, 0 }};
    uint32_t changes[2];

    wrong_position = BRIDGE_COMPONENT("game:Position", position_t,
        BRIDGE_STRIP, BRIDGE_TABLE);
    wrong_thing = BRIDGE_COMPONENT("other:Thing", marked_t, BRIDGE_STRIP,
        BRIDGE_TABLE);
    wrong_flag = BRIDGE_DECLARE_TAG("game:Flag", BRIDGE_STRIP, BRIDGE_TABLE);
    terms[0].slot = wrong_position;
    changes[0] = wrong_position;
    changes[1] = wrong_flag;
    BRIDGE_ADD_SYSTEM("act", "update", terms, 1, changes, 2);
    AT_INIT();
    return (0);
}
