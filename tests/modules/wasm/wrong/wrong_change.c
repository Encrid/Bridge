/*
** Copyright (C) 2026 Bridge Owner
**
** This file is part of Bridge.
**
** Under the GNU Affero General Public License, version 3 only: see license.
**
** tests/modules/wasm/wrong/wrong_change.c -- the wrong module: a change
** stated of a slot nobody declared or of what no grant lets it write, and
** an attach an entry refuses.
*/

#include "wrong.h"

/*
** wrong_change_slot_undeclared
**
** Registers a system that states a change of a slot never declared.
*/
void wrong_change_slot_undeclared(void)
{
    wrong_add_with_change(WRONG_NO_SLOT);
}

/*
** wrong_change_ungranted
**
** Registers a system that states a change of what it may only read.
*/
void wrong_change_ungranted(void)
{
    wrong_add_with_change(wrong_thing);
}

/*
** wrong_attach_undeclared
**
** Attaches a slot the module never declared.
*/
void wrong_attach_undeclared(void)
{
    bridge_attach(1, WRONG_NO_SLOT, BRIDGE_ZEROED);
}

/*
** wrong_attach_ungranted
**
** Attaches what the module may only read.
*/
void wrong_attach_ungranted(void)
{
    bridge_attach(1, wrong_thing, BRIDGE_ZEROED);
}

/*
** wrong_attach_outside
**
** Attaches the position with a value that is not in the scratch arena.
*/
void wrong_attach_outside(void)
{
    bridge_attach(1, wrong_position, WRONG_OUTSIDE);
}
