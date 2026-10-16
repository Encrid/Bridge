/*
** Copyright (C) 2026 Enkrid
**
** Under the GNU Affero General Public License, version 3 only: see license.
*/
#include "wrong.h"

void wrong_change_slot_undeclared(void)
{
    wrong_add_with_change(WRONG_NO_SLOT);
}

void wrong_change_ungranted(void)
{
    wrong_add_with_change(wrong_thing);
}

void wrong_attach_undeclared(void)
{
    bridge_attach(1, WRONG_NO_SLOT, BRIDGE_ZEROED);
}

void wrong_attach_ungranted(void)
{
    bridge_attach(1, wrong_thing, BRIDGE_ZEROED);
}

void wrong_attach_outside(void)
{
    bridge_attach(1, wrong_position, WRONG_OUTSIDE);
}
